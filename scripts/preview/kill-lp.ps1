# Kills slint-lsp instances + anything holding the live-preview control
# port (42123), then reports whether the port is free.
Get-Process slint-lsp -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Milliseconds 300
$conn = Get-NetTCPConnection -LocalPort 42123 -ErrorAction SilentlyContinue
if ($conn) {
    $conn | ForEach-Object { Stop-Process -Id $_.OwningProcess -Force -ErrorAction SilentlyContinue }
    Start-Sleep -Milliseconds 500
}
$still = Get-NetTCPConnection -LocalPort 42123 -ErrorAction SilentlyContinue
if ($still) { Write-Output "port busy" } else { Write-Output "port free" }
