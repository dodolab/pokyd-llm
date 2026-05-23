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
    set POKYD_LLM_HOST=10.0.2.2:8765
    powershell -ExecutionPolicy Bypass -File scripts\run-dosbox.ps1
    powershell -ExecutionPolicy Bypass -File scripts\run-dosbox.ps1 -SkipIntro
#>

[CmdletBinding()]
param(
    [switch] $ExitAfterNotes,
    [switch] $AllegroTest,
    [switch] $RawKey,
    [string] $LlmHost = "",
    [switch] $SkipIntro
)

$ErrorActionPreference = "Stop"
. "$PSScriptRoot\common.ps1"
. "$PSScriptRoot\pokyd-llm-env.ps1"
Initialize-NotesScriptsDir -ScriptsDirectory $PSScriptRoot

$RepoRoot = Get-NotesRepoRoot

if (-not $LlmHost -and (Test-PokydLlmConfigured)) {
    $LlmHost = Resolve-PokydLlmHost -RepoRoot $RepoRoot
}

if ($RawKey -and $AllegroTest) {
    Write-Error "Use only one of -RawKey or -AllegroTest."
}
if ($LlmHost -and ($RawKey -or $AllegroTest)) {
    Write-Error "-LlmHost cannot be combined with -RawKey or -AllegroTest."
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

if ($LlmHost) {
    $ne2000Com = Join-Path $RepoRoot "assets\NE2000.COM"
    if (-not (Test-Path -LiteralPath $ne2000Com)) {
        Write-Error "LLM mode needs assets\NE2000.COM. See scripts\download-ne2000.sh or bridge\README.md."
    }
}

$guestLaunchLine = $ExeToRun
if ($ExeToRun -eq "pokyd.exe") {
    $pArgs = @()
    if ($SkipIntro) { $pArgs += "-pokyd" }
    $pArgs += "-consplit"
    if ($LlmHost) { $pArgs += "-llm=$LlmHost" }
    $guestLaunchLine = "pokyd.exe " + ($pArgs -join " ")
}

$autoexecTail = @(
    "if not exist C:\SLOVNIK.DAT if exist C:\assets\slova.pkd copy C:\assets\slova.pkd C:\SLOVA.PKD >nul",
    "if not exist C:\SLOVNIK.DAT if exist C:\assets\pokydx.pkd copy C:\assets\pokydx.pkd C:\POKYDX.PKD >nul",
    "if not exist C:\SLOVNIK.DAT if exist C:\slova.exe slova.exe",
    "echo [pokyd] Starting $ExeToRun ...",
    $guestLaunchLine,
    "echo.",
    "echo $ExeToRun ended with errorlevel %ERRORLEVEL%",
    "echo Staying at C:\ prompt --- type EXIT to close DOSBox."
)
if ($ExitAfterNotes) {
    $autoexecTail = @($guestLaunchLine, "exit")
}

$ne2000Section = @()
$ne2000Autoexec = @()
if ($LlmHost) {
    $ne2000Section = @(
        "[ne2000]",
        "ne2000=true",
        "nicbase=0x300",
        "nicirq=10",
        "backend=slirp"
    )
    $ne2000Autoexec = @(
        "copy C:\assets\WATTCP.CFG C:\WATTCP.CFG >nul",
        "SET WATTCP.CFG=C:",
        "echo [pokyd] NE2000 assets\NE2000.COM 0x60 10 0x300",
        "C:\assets\NE2000.COM 0x60 10 0x300"
    )
}

$coreLine = "core=normal"
if ($LlmHost) {
    $coreOverride = $env:POKYD_DOSBOX_CPU_CORE
    if ($coreOverride) {
        $coreLine = "core=$coreOverride"
    } else {
        $coreLine = "core=dynamic"
    }
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
        elseif ($LlmHost) { "title=Pokyd DOS (LLM / Windows)" }
        else { "title=Pokyd DOS" }
    ),
    "keyboard hook=false",
    "[cpu]",
    $coreLine,
    "fpu=true",
    "[dos]",
    "ver=7.1",
    "lfn=true",
    "keyboardlayout=us"
) + $ne2000Section + @(
    "[autoexec]",
    "@echo off",
    "echo [pokyd] Autoexec started",
    "mount c `"$mountPath`"",
    "c:",
    "echo [pokyd] Switched to drive C",
    "SET PATH=C:\"
) + $ne2000Autoexec + $autoexecTail
$confPath = Join-Path $env:TEMP "pokyd-dosbox-x-$([Guid]::NewGuid().ToString('n')).conf"
$confLines | Set-Content -Path $confPath -Encoding ASCII

Write-Host "Launching: $db"
Write-Host "Guest EXE: $ExeToRun"
Write-Host "Config:    $confPath"
& $db -conf $confPath
$dbExit = $LASTEXITCODE
if ($null -eq $dbExit) { $dbExit = 0 }
exit $dbExit
