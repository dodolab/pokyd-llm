# Shared helpers for DOSBox-X scripts (repo root, toolchain layout, DOSBox-X discovery).
#
# Normal mode (no LLM / no slirp):
#   1) NOTES_DOSBOX_X, 2) dosbox-x on PATH, 3) repo dosbox\, 4) .tools\dosbox-x\ (VS build).
#
# LLM mode (NE2000 + slirp to reach the Node bridge at 10.0.2.2):
#   Visual Studio DOSBox-X builds do NOT include slirp on Windows. Use a MinGW build instead.
#   1) NOTES_DOSBOX_X if it passes Test-DosBoxXSupportsSlirp,
#   2) .tools\dosbox-x-llm\ (newer MinGW + SDL2; matches dosbox\ VS look),
#   3) .tools\dosbox-x-mingw\ (older auto-download fallback),
#   4) dosbox-x on PATH if slirp works,
#   5) auto-download MinGW win64 into .tools\dosbox-x-mingw\ (Install-DosBoxXPortable -RequireSlirp).

$script:PokydDosBoxXMingwZipUrl = "https://github.com/joncampbell123/dosbox-x/releases/download/dosbox-x-v2026.01.02/dosbox-x-mingw-win64-20260102233440.zip"
$script:PokydDosBoxXVsZipUrl = "https://github.com/joncampbell123/dosbox-x/releases/download/dosbox-x-v2026.03.29/dosbox-x-vsbuild-win64-2026.03.29-portable.zip"

function Find-DosBoxXExeInTree {
    param([Parameter(Mandatory)][string] $RootDir)
    if (-not (Test-Path -LiteralPath $RootDir)) { return $null }
    $direct = Join-Path $RootDir "dosbox-x.exe"
    if (Test-Path -LiteralPath $direct) { return $direct }
    $found = @(Get-ChildItem -LiteralPath $RootDir -Recurse -Filter "dosbox-x.exe" -ErrorAction SilentlyContinue)
    if (-not $found.Count) { return $null }
  # Prefer mingw-sdl2 (same SDL2 renderer as the VS build in dosbox\; wider window / aspect).
    $pick = $found | Sort-Object @{
        Expression = {
            $p = $_.FullName.ToLower()
            if ($p -match 'mingw-sdl2') { 0 }
            elseif ($p -match 'mingw-build\\mingw\\') { 1 }
            else { 2 }
        }
    }, @{ Expression = { $_.FullName.Length } } | Select-Object -First 1
    return $pick.FullName
}

function Initialize-NotesScriptsDir {
    param([Parameter(Mandatory)][string] $ScriptsDirectory)
    $script:NotesScriptsDir = $ScriptsDirectory
}

function Get-NotesRepoRoot {
    if (-not $script:NotesScriptsDir) {
        throw "Initialize-NotesScriptsDir must be called first (pass `$PSScriptRoot from the caller script)."
    }
    (Resolve-Path (Join-Path $script:NotesScriptsDir "..")).Path
}

function Get-DosBoxXProcessIds {
    @(Get-Process -Name 'dosbox-x' -ErrorAction SilentlyContinue | ForEach-Object { $_.Id })
}

function Wait-DosBoxXSession {
    param(
        [Parameter(Mandatory)][string] $ExePath,
        [Parameter(Mandatory)][string] $ConfPath
    )

    $before = @(Get-DosBoxXProcessIds)
    # Direct invocation: Start-Process can drop -conf on some DOSBox-X builds (no autoexec).
    & $ExePath -conf $ConfPath
    $launcherExit = $LASTEXITCODE

    $deadline = [datetime]::UtcNow.AddSeconds(15)
    while ([datetime]::UtcNow -lt $deadline) {
        $sessionPids = @(Get-DosBoxXProcessIds | Where-Object { $before -notcontains $_ })
        if ($sessionPids.Count -gt 0) { break }
        Start-Sleep -Milliseconds 250
    }

    foreach ($sessionPid in $sessionPids) {
        Wait-Process -Id $sessionPid -ErrorAction SilentlyContinue
    }

    if ($sessionPids.Count -gt 0) { return 0 }
    if ($null -ne $launcherExit) { return $launcherExit }
    return 0
}

function Test-DosBoxXTrustedForSlirp {
    param([Parameter(Mandatory)][string] $ExePath)
    $norm = $ExePath.ToLower().Replace('/', '\')
    return ($norm -match '\\\.tools\\dosbox-x-llm\\' -or $norm -match '\\\.tools\\dosbox-x-mingw\\')
}

function Test-DosBoxXSupportsSlirp {
    param([Parameter(Mandatory)][string] $ExePath)

    if (-not (Test-Path -LiteralPath $ExePath)) { return $false }

    $logPath = Join-Path $env:TEMP ("pokyd-slirp-probe-{0}.log" -f [Guid]::NewGuid().ToString('n'))
    $confPath = Join-Path $env:TEMP ("pokyd-slirp-probe-{0}.conf" -f [Guid]::NewGuid().ToString('n'))
    try {
        @(
            "[dosbox]",
            "title=slirp probe",
            "[ne2000]",
            "ne2000=true",
            "nicbase=0x300",
            "nicirq=10",
            "backend=slirp",
            "[log]",
            "logfile=$logPath",
            "[autoexec]",
            "@echo off",
            "exit"
        ) | Set-Content -Path $confPath -Encoding ASCII

        Remove-Item -LiteralPath $logPath -Force -ErrorAction SilentlyContinue
        $proc = Start-Process -FilePath $ExePath -ArgumentList @("-conf", $confPath, "-noconsole", "-faststart", "-fastlaunch") `
            -PassThru -WindowStyle Hidden
        if ($proc) {
            $proc.WaitForExit(15000) | Out-Null
            if (-not $proc.HasExited) {
                Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
            }
        }
        Start-Sleep -Milliseconds 300

        if (-not (Test-Path -LiteralPath $logPath)) { return $false }
        $logText = Get-Content -LiteralPath $logPath -Raw -ErrorAction SilentlyContinue
        if (-not $logText) { return $false }
        if ($logText -match 'Backend not supported in this build:\s*slirp') { return $false }
        if ($logText -match 'Failed to open Ethernet backend slirp') { return $false }
        return ($logText -match 'NE2000 Ethernet emulation backend selected:\s*slirp')
    } finally {
        Remove-Item -LiteralPath $logPath -Force -ErrorAction SilentlyContinue
        Remove-Item -LiteralPath $confPath -Force -ErrorAction SilentlyContinue
    }
}

function Find-DosBoxX {
    param(
        [string] $RepoRoot = (Get-NotesRepoRoot),
        [switch] $RequireSlirp
    )

    $candidates = @()

    if ($env:NOTES_DOSBOX_X -and (Test-Path -LiteralPath $env:NOTES_DOSBOX_X)) {
        $candidates += $env:NOTES_DOSBOX_X
    }

    if ($RequireSlirp) {
        foreach ($root in @(
            (Join-Path $RepoRoot ".tools\dosbox-x-llm"),
            (Join-Path $RepoRoot ".tools\dosbox-x-mingw")
        )) {
            $exe = Find-DosBoxXExeInTree -RootDir $root
            if ($exe) { $candidates += $exe }
        }

        $cmd = Get-Command "dosbox-x" -ErrorAction SilentlyContinue
        if ($cmd) { $candidates += $cmd.Source }
    } else {
        $cmd = Get-Command "dosbox-x" -ErrorAction SilentlyContinue
        if ($cmd) { $candidates += $cmd.Source }

        $inBundled = Find-DosBoxXExeInTree -RootDir (Join-Path $RepoRoot "dosbox")
        if ($inBundled) { $candidates += $inBundled }

        $inTools = Find-DosBoxXExeInTree -RootDir (Join-Path $RepoRoot ".tools\dosbox-x")
        if ($inTools) { $candidates += $inTools }
    }

    $seen = @{}
    foreach ($exe in $candidates) {
        if (-not $exe -or $seen.ContainsKey($exe)) { continue }
        $seen[$exe] = $true
        if ($RequireSlirp) {
            if (Test-DosBoxXTrustedForSlirp -ExePath $exe) { return $exe }
            if (Test-DosBoxXSupportsSlirp -ExePath $exe) { return $exe }
        } else {
            return $exe
        }
    }

    if ($RequireSlirp -and $env:NOTES_DOSBOX_X -and (Test-Path -LiteralPath $env:NOTES_DOSBOX_X)) {
        throw @"
NOTES_DOSBOX_X is set to a DOSBox-X build without NE2000/slirp support:
  $($env:NOTES_DOSBOX_X)

LLM mode needs the MinGW win64 DOSBox-X build (slirp backend). Either unset NOTES_DOSBOX_X and re-run,
or point it at a MinGW dosbox-x.exe (see README Windows / LLM section).
"@
    }

    return $null
}

function Install-DosBoxXPortable {
    param(
        [string] $RepoRoot = (Get-NotesRepoRoot),
        [switch] $RequireSlirp
    )

    if ($RequireSlirp) {
        $DosBoxDir = Join-Path $RepoRoot ".tools\dosbox-x-mingw"
        Write-Host "DOSBox-X MinGW (slirp/NE2000) not found. Downloading portable win64 build..."
        New-Item -ItemType Directory -Force -Path $DosBoxDir | Out-Null
        $zipPath = Join-Path $DosBoxDir "dosbox-x-mingw-win64.zip"
        Invoke-WebRequest -Uri $script:PokydDosBoxXMingwZipUrl -OutFile $zipPath -UseBasicParsing
        Expand-Archive -Path $zipPath -DestinationPath $DosBoxDir -Force
        Remove-Item $zipPath -Force -ErrorAction SilentlyContinue

        $db = Find-DosBoxX -RepoRoot $RepoRoot -RequireSlirp
        if (-not $db) { throw "Could not locate a slirp-capable dosbox-x.exe after MinGW extract." }
        Write-Host "Using slirp-capable DOSBox-X: $db"
        return $db
    }

    $DosBoxDir = Join-Path $RepoRoot ".tools\dosbox-x"
    Write-Host "DOSBox-X not found. Downloading portable VS win64 build..."
    New-Item -ItemType Directory -Force -Path $DosBoxDir | Out-Null
    $zipPath = Join-Path $DosBoxDir "dosbox-x-portable.zip"
    Invoke-WebRequest -Uri $script:PokydDosBoxXVsZipUrl -OutFile $zipPath -UseBasicParsing
    Expand-Archive -Path $zipPath -DestinationPath $DosBoxDir -Force
    Remove-Item $zipPath -Force -ErrorAction SilentlyContinue

    $db = Find-DosBoxX -RepoRoot $RepoRoot
    if (-not $db) { throw "Could not locate dosbox-x.exe after extract." }
    return $db
}
