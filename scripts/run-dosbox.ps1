<#
.SYNOPSIS
  Run pokyd.exe inside DOSBox-X with the repo mounted as C:.

.DESCRIPTION
  - Uses DOSBox-X for reliable long filenames on a Windows-mounted folder.
  - Resolves dosbox-x.exe via scripts/common.ps1 (NOTES_DOSBOX_X, PATH, repo dosbox\, then .tools\dosbox-x\).
  - LLM mode (-llm / Watt-32): requires MinGW DOSBox-X with NE2000 slirp (NOT the VS build in dosbox\).
    Auto-downloads into .tools\dosbox-x-mingw\ when missing.
  - Non-LLM fallback: portable VS win64 ZIP in .tools\dosbox-x\.

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
    [switch] $SkipIntro,
    [switch] $EnableLlm
)

$ErrorActionPreference = "Stop"
. "$PSScriptRoot\common.ps1"
. "$PSScriptRoot\pokyd-llm-env.ps1"
Initialize-NotesScriptsDir -ScriptsDirectory $PSScriptRoot

$RepoRoot = Get-NotesRepoRoot

if ($EnableLlm -and -not $LlmHost) {
    $LlmHost = Resolve-PokydLlmHost -RepoRoot $RepoRoot
}
elseif (-not $LlmHost -and (Test-PokydLlmConfigured)) {
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

if ($LlmHost) {
    $db = Find-DosBoxX -RepoRoot $RepoRoot -RequireSlirp
    if (-not $db) { $db = Install-DosBoxXPortable -RepoRoot $RepoRoot -RequireSlirp }
} else {
    $db = Find-DosBoxX -RepoRoot $RepoRoot
    if (-not $db) { $db = Install-DosBoxXPortable -RepoRoot $RepoRoot }
}

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
    if ($LlmHost) {
        # DOS COMMAND.COM splits on ':' (drive labels); pass host and port separately.
        if ($LlmHost -match '^([^:]+):(\d+)$') {
            $pArgs += "-llm"
            $pArgs += $Matches[1]
            $pArgs += $Matches[2]
        } else {
            $pArgs += "-llm=$LlmHost"
        }
    }
    $guestLaunchLine = "pokyd.exe " + ($pArgs -join " ")
}

$autoexecTail = @(
    "if not exist C:\SLOVNIK.DAT if exist C:\assets\slova.pkd copy C:\assets\slova.pkd C:\SLOVA.PKD >nul",
    "if not exist C:\SLOVNIK.DAT if exist C:\assets\pokydx.pkd copy C:\assets\pokydx.pkd C:\POKYDX.PKD >nul",
    "if not exist C:\SLOVNIK.DAT if exist C:\slova.exe slova.exe",
    "echo [pokyd] Starting $ExeToRun ...",
    "echo [pokyd] Command: $guestLaunchLine",
    $guestLaunchLine,
    "echo.",
    "echo $ExeToRun ended with errorlevel %ERRORLEVEL%"
)
if ($LlmHost) {
    $autoexecTail += "echo Re-run LLM mode: POKYDLLM.BAT"
}
$autoexecTail += "echo Staying at C:\ prompt --- type EXIT to close DOSBox."
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
        "backend=slirp",
        "macaddr=AC:DE:48:88:99:01"
    )
    $ne2000Autoexec = @(
        "copy C:\assets\WATTCP.CFG C:\WATTCP.CFG >nul",
        "SET WATTCP.CFG=C:",
        "echo [pokyd] NE2000 assets\NE2000.COM 0x60 10 0x300",
        "echo [pokyd] If MAC is FF:FF:FF:FF:FF:FF below, slirp/LLM will not work.",
        "C:\assets\NE2000.COM 0x60 10 0x300",
        "if errorlevel 1 echo [pokyd] WARNING: NE2000.COM returned errorlevel %ERRORLEVEL%"
    )
}

$slirpSection = @()
if ($LlmHost) {
    $slirpSection = @(
        "[ethernet, slirp]",
        "disable_host_loopback=false",
        "restricted=false"
    )
}

$coreLine = "core=normal"
$cputypeLine = @()
if ($LlmHost) {
    $coreOverride = $env:POKYD_DOSBOX_CPU_CORE
    if ($coreOverride) {
        $coreLine = "core=$coreOverride"
    } else {
        $coreLine = "core=dynamic"
    }
    $cputypeLine = @("cputype=pentium")
}

$sdlOutput = "surface"
if ($LlmHost -and $db -match 'mingw-sdl2') {
    $sdlOutput = "opengl"
}

$confLines = @(
    "[sdl]",
    "output=$sdlOutput",
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
    $coreLine
) + $cputypeLine + @(
    "fpu=true",
    "[dos]",
    "ver=7.1",
    "lfn=true",
    "keyboardlayout=us"
) + $ne2000Section + $slirpSection + @(
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
if ($LlmHost) {
    Write-Host "Network:   NE2000 + slirp (host bridge at $LlmHost)"
    if ($db -match 'dosbox-x-llm') { Write-Host "Emulator:  .tools/dosbox-x-llm (MinGW SDL2, slirp)" }
    elseif ($db -match 'mingw-sdl2') { Write-Host "Emulator:  MinGW SDL2 build (slirp)" }
}
Write-Host "Guest EXE: $ExeToRun"
Write-Host "Config:    $confPath"
Write-Host "Waiting for DOSBox-X to close (PowerShell stays blocked until you type EXIT in DOSBox)..."
$dbExit = Wait-DosBoxXSession -ExePath $db -ConfPath $confPath
if ($null -eq $dbExit) { $dbExit = 0 }
exit $dbExit
