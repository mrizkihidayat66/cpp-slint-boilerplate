# capture-all.ps1 — full verification suite.
#
# Captures every page at multiple window sizes plus scrolled gallery
# sections, into build/shots/<name>.png. Each capture launches a fresh
# process so state never leaks between shots.

param(
    [string]$Exe = "D:\Development\wip\cpp-slint-boilerplate\build\x64-debug\slint-boilerplate.exe",
    [string]$OutDir = "D:\Development\wip\cpp-slint-boilerplate\build\shots"
)

$ErrorActionPreference = "Stop"
$harness = "$PSScriptRoot\capture-ui.ps1"

# name, page, W, H, scroll
$shots = @(
    # Dashboard at three sizes
    @{ n = "dashboard-1024";   p = 0; w = 1024; h = 680;  s = -1 },
    @{ n = "dashboard-800";    p = 0; w = 800;  h = 600;  s = -1 },
    @{ n = "dashboard-1280";   p = 0; w = 1280; h = 800;  s = -1 },
    @{ n = "dashboard-min";    p = 0; w = 480;  h = 360;  s = -1 },
    # Tasks (table, toolbar, pagination)
    @{ n = "tasks-1024";       p = 1; w = 1024; h = 680;  s = -1 },
    @{ n = "tasks-800";        p = 1; w = 800;  h = 600;  s = -1 },
    @{ n = "tasks-min";        p = 1; w = 480;  h = 360;  s = -1 },
    # Gallery: top at several sizes (responsive grid proof)
    @{ n = "gallery-1280";     p = 2; w = 1280; h = 800;  s = -1 },
    @{ n = "gallery-1024";     p = 2; w = 1024; h = 680;  s = -1 },
    @{ n = "gallery-800";      p = 2; w = 800;  h = 600;  s = -1 },
    @{ n = "gallery-min";      p = 2; w = 480;  h = 360;  s = -1 },
    # Gallery scrolled to the new sections (fractions of scrollable range)
    @{ n = "gallery-buttons2"; p = 2; w = 1024; h = 680;  s = 0.30 },
    @{ n = "gallery-containers"; p = 2; w = 1024; h = 680; s = 0.42 },
    @{ n = "gallery-datepicker"; p = 2; w = 1024; h = 680; s = 0.55 },
    @{ n = "gallery-grid";     p = 2; w = 1024; h = 680;  s = 0.72 },
    @{ n = "gallery-binding";  p = 2; w = 1024; h = 680;  s = 0.92 },
    # Settings / About
    @{ n = "settings-1024";    p = 3; w = 1024; h = 680;  s = -1 },
    @{ n = "about-1024";       p = 4; w = 1024; h = 680;  s = -1 },
    @{ n = "about-800";        p = 4; w = 800;  h = 600;  s = -1 }
)

foreach ($shot in $shots) {
    $out = Join-Path $OutDir "$($shot.n).png"
    $args = @("-File", $harness, "-Exe", $Exe, "-Out", $out,
              "-W", $shot.w, "-H", $shot.h, "-Page", $shot.p)
    if ($shot.s -ge 0) { $args += @("-Scroll", $shot.s) }
    Write-Host "=== $($shot.n) ($($shot.w)x$($shot.h)) ===" -ForegroundColor Cyan
    & powershell @args
    if ($LASTEXITCODE -ne 0 -and $LASTEXITCODE -ne $null) {
        Write-Host "capture failed: $($shot.n)" -ForegroundColor Red
    }
}
Write-Host "done: $($shots.Count) shots in $OutDir" -ForegroundColor Green
