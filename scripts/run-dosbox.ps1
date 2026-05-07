<#
.SYNOPSIS
  Run pokyd.exe inside DOSBox-X with the repo mounted as C:.

.DESCRIPTION
  - Uses DOSBox-X for reliable long filenames on a Windows-mounted folder.
  - Resolves dosbox-x.exe via scripts/common.ps1 (NOTES_DOSBOX_X, PATH, repo dosbox\, then .tools\dosbox-x\).
  - If still not found, downloads the portable win64 ZIP into .tools/dosbox-x/ (optional fallback).

  Build on Windows first: build.bat (Open Watcom). This script only runs the DOS binary.

  By default the window stays at the C:\> prompt after pokyd.exe so you can read messages
  or re-run Pokyd. Use -ExitAfterNotes to run "exit" automatically.

  Usage (from repo root):
    powershell -ExecutionPolicy Bypass -File scripts\run-dosbox.ps1
    powershell -ExecutionPolicy Bypass -File scripts\run-dosbox.ps1 -ExitAfterNotes
    powershell -ExecutionPolicy Bypass -File scripts\run-dosbox.ps1 -AllegroTest
    powershell -ExecutionPolicy Bypass -File scripts\run-dosbox.ps1 -RawKey
#>

[CmdletBinding()]
param(
    [switch] $ExitAfterNotes,
    [switch] $AllegroTest,
    [switch] $RawKey
)

$ErrorActionPreference = "Stop"
. "$PSScriptRoot\common.ps1"
Initialize-NotesScriptsDir -ScriptsDirectory $PSScriptRoot

$RepoRoot = Get-NotesRepoRoot

if ($RawKey -and $AllegroTest) {
    Write-Error "Use only one of -RawKey or -AllegroTest."
}

$ExeToRun = if ($RawKey) { "rawkey.exe" } elseif ($AllegroTest) { "test.exe" } else { "pokyd.exe" }
$GuestExe = Join-Path $RepoRoot $ExeToRun

if (-not (Test-Path $GuestExe)) {
    if ($RawKey) {
        Write-Error "rawkey.exe not found at $GuestExe. Run build-rawkey.bat first."
    } elseif ($AllegroTest) {
        Write-Error "test.exe not found at $GuestExe. Run build-test.bat first."
    } else {
        Write-Error "pokyd.exe not found at $GuestExe. Run build.bat (Open Watcom on Windows) first."
    }
}

$db = Find-DosBoxX -RepoRoot $RepoRoot
if (-not $db) { $db = Install-DosBoxXPortable -RepoRoot $RepoRoot }

$mountPath = $RepoRoot
$autoexecTail = @(
    "if not exist C:\SLOVNIK.DAT if exist C:\assets\slova.pkd copy C:\assets\slova.pkd C:\SLOVA.PKD >nul",
    "if not exist C:\SLOVNIK.DAT if exist C:\assets\pokydx.pkd copy C:\assets\pokydx.pkd C:\POKYDX.PKD >nul",
    "if not exist C:\SLOVNIK.DAT if exist C:\slova.exe slova.exe",
    "echo [pokyd] Starting $ExeToRun ...",
    $ExeToRun,
    "echo.",
    "echo $ExeToRun ended with errorlevel %ERRORLEVEL%",
    "echo Staying at C:\ prompt --- type EXIT to close DOSBox."
)
if ($ExitAfterNotes) {
    $autoexecTail = @($ExeToRun, "exit")
}

$confLines = @(
    "[sdl]",
    "output=surface",
    "usescancodes=false",
    "[render]",
    "aspect=true",
    "doublescan=false",
    "[dosbox]",
    $(
        if ($RawKey) { "title=RAW conio keyboard (rawkey.exe)" }
        elseif ($AllegroTest) { "title=Allegro keyboard test (test.exe)" }
        else { "title=Pokyd DOS" }
    ),
    "keyboard hook=false",
    "[cpu]",
    "core=normal",
    "[dos]",
    "ver=7.1",
    "lfn=true",
    "keyboardlayout=us",
    "[autoexec]",
    "@echo off",
    "echo [pokyd] Autoexec started",
    "mount c `"$mountPath`"",
    "c:",
    "echo [pokyd] Switched to drive C",
    "SET PATH=C:\"
) + $autoexecTail
$confPath = Join-Path $env:TEMP "pokyd-dosbox-x-$([Guid]::NewGuid().ToString('n')).conf"
$confLines | Set-Content -Path $confPath -Encoding ASCII

Write-Host "Launching: $db"
Write-Host "Guest EXE: $ExeToRun"
Write-Host "Config:    $confPath"
& $db -conf $confPath
