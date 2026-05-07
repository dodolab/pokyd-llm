$ErrorActionPreference = "Stop"
. "$PSScriptRoot\common.ps1"
Initialize-NotesScriptsDir -ScriptsDirectory $PSScriptRoot
$RepoRoot = Get-NotesRepoRoot

Remove-Item -Path (Join-Path $RepoRoot "pokyd.exe") -Force -ErrorAction SilentlyContinue
Get-ChildItem -Path $RepoRoot -Filter "*.obj" -ErrorAction SilentlyContinue | Remove-Item -Force
Get-ChildItem -Path (Join-Path $RepoRoot "src") -Filter "*.obj" -ErrorAction SilentlyContinue | Remove-Item -Force
Remove-Item -Path (Join-Path $RepoRoot "debug.log") -Force -ErrorAction SilentlyContinue
Write-Host "Clean done."
