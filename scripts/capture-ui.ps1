# capture-ui.ps1 — precision UI capture harness.
#
# Launches the boilerplate exe, sizes its window to an exact client size
# (DPI-aware), lets it render, then captures the *client area* via
# PrintWindow (GPU-rendered content included), cropping away the title bar
# and borders that GetWindowRect-based captures suffered from.
#
# Usage: powershell -File capture-ui.ps1 -Exe <path> -Out <png> [-W 1024] [-H 680]
#        [-Page 0..4] [-Scroll 0..1]
param(
    [Parameter(Mandatory = $true)][string]$Exe,
    [Parameter(Mandatory = $true)][string]$Out,
    [int]$W = 1024,
    [int]$H = 680,
    [int]$Page = -1,
    [double]$Scroll = -1
)

$ErrorActionPreference = "Stop"
Add-Type -AssemblyName System.Drawing

Add-Type @"
using System;
using System.Runtime.InteropServices;

public class Win32Cap {
    [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
    [DllImport("user32.dll")] public static extern bool EnumWindows(EnumWindowsProc cb, IntPtr lParam);
    [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr hWnd, out uint pid);
    [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr hWnd, out RECT rect);
    [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr hWnd, out POINT pt);
    [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr hWnd, IntPtr hdc, uint flags);
    [DllImport("user32.dll")] public static extern bool AdjustWindowRectEx(ref RECT rect, uint style, bool menu, uint exStyle);
    [DllImport("user32.dll")] public static extern uint GetWindowLong(IntPtr hWnd, int index);
    [DllImport("user32.dll")] public static extern bool SetWindowPos(IntPtr hWnd, IntPtr after, int x, int y, int w, int h, uint flags);
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);

    // Screen capture (BitBlt) for windows PrintWindow can't render.
    [DllImport("gdi32.dll")] public static extern bool BitBlt(IntPtr hdcDest, int x, int y, int w, int h, IntPtr hdcSrc, int x1, int y1, uint rop);
    [DllImport("user32.dll")] public static extern IntPtr GetDC(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern int ReleaseDC(IntPtr hWnd, IntPtr hdc);
    [DllImport("user32.dll")] public static extern IntPtr GetWindowDC(IntPtr hWnd);

    public delegate bool EnumWindowsProc(IntPtr hWnd, IntPtr lParam);

    [StructLayout(LayoutKind.Sequential)]
    public struct RECT { public int Left, Top, Right, Bottom; }

    [StructLayout(LayoutKind.Sequential)]
    public struct POINT { public int X, Y; }
}
"@

[Win32Cap]::SetProcessDPIAware() | Out-Null

# --- Launch with harness env vars -----------------------------------------
$env:BOILERPLATE_START_PAGE = if ($Page -ge 0) { "$Page" } else { $null }
$env:BOILERPLATE_GALLERY_SCROLL = if ($Scroll -ge 0) { "$Scroll" } else { $null }

$psi = New-Object System.Diagnostics.ProcessStartInfo
$psi.FileName = $Exe
$psi.WorkingDirectory = Split-Path $Exe
$psi.UseShellExecute = $true  # env vars flow from this process
$proc = [System.Diagnostics.Process]::Start($psi)

try {
    # Wait for a visible top-level window (up to 15 s).
    $hwnd = [IntPtr]::Zero
    $deadline = [DateTime]::UtcNow.AddSeconds(15)
    while ([DateTime]::UtcNow -lt $deadline -and $hwnd -eq [IntPtr]::Zero) {
        Start-Sleep -Milliseconds 200
        $found = [IntPtr]::Zero
        $cb = [Win32Cap+EnumWindowsProc] {
            param($h, $l)
            $wpid = 0
            [Win32Cap]::GetWindowThreadProcessId($h, [ref]$wpid) | Out-Null
            if ($wpid -eq $proc.Id -and [Win32Cap]::IsWindowVisible($h)) {
                $r = New-Object Win32Cap+RECT
                [Win32Cap]::GetClientRect($h, [ref]$r) | Out-Null
                if (($r.Right - $r.Left) -gt 100 -and ($r.Bottom - $r.Top) -gt 80) {
                    $script:found = $h
                    return $false
                }
            }
            return $true
        }
        [Win32Cap]::EnumWindows($cb, [IntPtr]::Zero) | Out-Null
        $hwnd = $found
    }
    if ($hwnd -eq [IntPtr]::Zero) { throw "no window found for pid $($proc.Id)" }

    # --- Size the window so the CLIENT area is exactly W x H ----------------
    $style = [Win32Cap]::GetWindowLong($hwnd, -16)   # GWL_STYLE
    $exStyle = [Win32Cap]::GetWindowLong($hwnd, -20) # GWL_EXSTYLE
    $frame = New-Object Win32Cap+RECT
    $frame.Left = 0; $frame.Top = 0; $frame.Right = $W; $frame.Bottom = $H
    [Win32Cap]::AdjustWindowRectEx([ref]$frame, $style, $false, $exStyle) | Out-Null
    $outerW = $frame.Right - $frame.Left
    $outerH = $frame.Bottom - $frame.Top

    # Two passes: DPI scaling makes AdjustWindowRectEx an approximation;
    # measure the resulting client rect and correct the outer size by the
    # deficit so the second pass lands on the exact target.
    for ($pass = 0; $pass -lt 2; $pass++) {
        [Win32Cap]::SetWindowPos($hwnd, [IntPtr]::Zero, 40, 40, $outerW, $outerH, 0x0004) | Out-Null
        Start-Sleep -Milliseconds 60
        $m = New-Object Win32Cap+RECT
        [Win32Cap]::GetClientRect($hwnd, [ref]$m) | Out-Null
        $dw = ($m.Right - $m.Left) - $W
        $dh = ($m.Bottom - $m.Top) - $H
        if ($dw -eq 0 -and $dh -eq 0) { break }
        $outerW -= $dw
        $outerH -= $dh
    }
    [Win32Cap]::SetForegroundWindow($hwnd) | Out-Null

    Start-Sleep -Milliseconds 900  # let the renderer settle at the new size

    # --- Capture the client area exactly -------------------------------------
    $client = New-Object Win32Cap+RECT
    [Win32Cap]::GetClientRect($hwnd, [ref]$client) | Out-Null
    $cw = $client.Right - $client.Left
    $ch = $client.Bottom - $client.Top

    $origin = New-Object Win32Cap+POINT
    [Win32Cap]::ClientToScreen($hwnd, [ref]$origin) | Out-Null
    # Window position was set to (40,40): client offset = origin - window pos.
    $clientX = $origin.X - 40
    $clientY = $origin.Y - 40

    # 1) Screen BitBlt of the exact client rect — the most reliable for
    #    GPU-rendered (femtovg) windows, as long as we are foreground.
    $crop = New-Object System.Drawing.Bitmap($cw, $ch)
    $cg = [System.Drawing.Graphics]::FromImage($crop)
    $cg.CopyFromScreen($origin.X, $origin.Y, 0, 0, (New-Object System.Drawing.Size($cw, $ch)))
    $cg.Dispose()

    # 2) Sanity check: if the capture is a flat fill, try PrintWindow into the
    #    full window bitmap and crop the client rect out of it instead.
    $flat = $true
    $base = $crop.GetPixel(0, 0).ToArgb()
    for ($x = 0; $x -lt $cw -and $flat; $x += 11) {
        for ($y = 0; $y -lt $ch; $y += 11) {
            if ($crop.GetPixel($x, $y).ToArgb() -ne $base) { $flat = $false; break }
        }
    }
    if ($flat) {
        $crop.Dispose()
        $fullW = [Math]::Max($outerW, $cw + 64)
        $fullH = [Math]::Max($outerH, $ch + 64)
        $bmp = New-Object System.Drawing.Bitmap($fullW, $fullH)
        $g = [System.Drawing.Graphics]::FromImage($bmp)
        $hdc = $g.GetHdc()
        [Win32Cap]::PrintWindow($hwnd, $hdc, 2) | Out-Null  # PW_RENDERFULLCONTENT
        $g.ReleaseHdc($hdc)
        $g.Dispose()
        $crop = New-Object System.Drawing.Bitmap($cw, $ch)
        $cg2 = [System.Drawing.Graphics]::FromImage($crop)
        $cg2.DrawImage($bmp, (New-Object System.Drawing.Rectangle(0, 0, $cw, $ch)),
            (New-Object System.Drawing.Rectangle($clientX, $clientY, $cw, $ch)),
            [System.Drawing.GraphicsUnit]::Pixel)
        $cg2.Dispose()
        $bmp.Dispose()
        Write-Host "captured via PrintWindow fallback"
    } else {
        Write-Host "captured via screen BitBlt"
    }

    New-Item -ItemType Directory -Force (Split-Path $Out) | Out-Null
    $crop.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
    $crop.Dispose()
    Write-Host "saved $Out (${cw}x${ch})"
} finally {
    if (!$proc.HasExited) { $proc.Kill() }
}
