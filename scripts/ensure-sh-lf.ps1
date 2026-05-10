# Force LF line endings in a shell script (Docker bind-mount + Linux bash breaks on CRLF).
param(
    [Parameter(Mandatory)]
    [string] $Path
)
$ErrorActionPreference = "Stop"
if (-not (Test-Path -LiteralPath $Path)) {
    Write-Error "File not found: $Path"
}
$bytes = [System.IO.File]::ReadAllBytes($Path)
if ($bytes.Length -eq 0) { return }
$content = [System.Text.Encoding]::UTF8.GetString($bytes)
$norm = $content -replace "`r`n", "`n" -replace "`r", "`n"
if ($content -ne $norm) {
    $enc = New-Object System.Text.UTF8Encoding $false
    [System.IO.File]::WriteAllText($Path, $norm, $enc)
    Write-Host "[bootstrap] Normalized CRLF to LF: $Path"
}
