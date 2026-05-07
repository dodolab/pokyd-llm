# Shared helpers for DOSBox-X scripts (repo root, toolchain layout, DOSBox-X discovery).
#
# Where DOSBox-X is resolved (edit Find-DosBoxX below to change defaults):
#   1) Environment variable NOTES_DOSBOX_X — full path to dosbox-x.exe (highest priority).
#   2) dosbox-x on PATH (Get-Command).
#   3) Repo-local folder dosbox\ — dosbox-x.exe at its root or anywhere under it (recursive).
#   4) .tools\dosbox-x\ — portable copy or auto-download target (see Install-DosBoxXPortable).

function Find-DosBoxXExeInTree {
    param([Parameter(Mandatory)][string] $RootDir)
    if (-not (Test-Path -LiteralPath $RootDir)) { return $null }
    $direct = Join-Path $RootDir "dosbox-x.exe"
    if (Test-Path -LiteralPath $direct) { return $direct }
    $found = Get-ChildItem -LiteralPath $RootDir -Recurse -Filter "dosbox-x.exe" -ErrorAction SilentlyContinue |
        Select-Object -First 1
    if ($found) { return $found.FullName }
    return $null
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

function Find-DosBoxX {
    param([string]$RepoRoot = (Get-NotesRepoRoot))
    $envExe = $env:NOTES_DOSBOX_X
    if ($envExe -and (Test-Path -LiteralPath $envExe)) { return $envExe }

    $cmd = Get-Command "dosbox-x" -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }

    $bundledRoot = Join-Path $RepoRoot "dosbox"
    $inBundled = Find-DosBoxXExeInTree -RootDir $bundledRoot
    if ($inBundled) { return $inBundled }

    $toolsRoot = Join-Path $RepoRoot ".tools\dosbox-x"
    $inTools = Find-DosBoxXExeInTree -RootDir $toolsRoot
    if ($inTools) { return $inTools }

    return $null
}

function Install-DosBoxXPortable {
    param([string]$RepoRoot = (Get-NotesRepoRoot))

    $DosBoxDir = Join-Path $RepoRoot ".tools\dosbox-x"
    Write-Host "DOSBox-X not found. Downloading portable win64 build..."
    New-Item -ItemType Directory -Force -Path $DosBoxDir | Out-Null
    $zipUrl = "https://github.com/joncampbell123/dosbox-x/releases/download/dosbox-x-v2026.03.29/dosbox-x-vsbuild-win64-2026.03.29-portable.zip"
    $zipPath = Join-Path $DosBoxDir "dosbox-x-portable.zip"
    Invoke-WebRequest -Uri $zipUrl -OutFile $zipPath -UseBasicParsing
    Expand-Archive -Path $zipPath -DestinationPath $DosBoxDir -Force
    Remove-Item $zipPath -Force -ErrorAction SilentlyContinue

    $db = Find-DosBoxX -RepoRoot $RepoRoot
    if (-not $db) { throw "Could not locate dosbox-x.exe after extract." }
    return $db
}
