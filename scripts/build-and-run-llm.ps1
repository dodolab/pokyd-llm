<#
.SYNOPSIS
  Build Pokyd with Watt-32 LLM support, start the Node OpenAI bridge, then launch DOSBox-X with NE2000 (slirp).

.DESCRIPTION
  Windows equivalent of build-and-run-llm.sh (macOS). Requires Node 18+, bridge/.env with OPENAI_API_KEY,
  vendor/watt32-dos (see build.sh), assets/NE2000.COM for the guest packet driver, and a MinGW DOSBox-X
  build with NE2000/slirp (auto-downloaded to .tools/dosbox-x-mingw/ if missing).

  Optional args (also accepted from build-and-run-llm.bat):
    -ExitAfterPokyd   Close DOSBox-X when pokyd.exe exits (automation).
    -SkipIntro        Pass -pokyd to skip the intro.

  Network env (same as macOS): BRIDGE_PORT, POKYD_LLM_IP, POKYD_LLM_PORT, POKYD_LLM_HOST.

.EXAMPLE
  powershell -ExecutionPolicy Bypass -File scripts\build-and-run-llm.ps1
  powershell -ExecutionPolicy Bypass -File scripts\build-and-run-llm.ps1 -ExitAfterPokyd
#>

[CmdletBinding()]
param(
    [switch] $ExitAfterPokyd,
    [switch] $SkipIntro
)

$ErrorActionPreference = "Stop"

. "$PSScriptRoot\common.ps1"
. "$PSScriptRoot\pokyd-llm-env.ps1"
Initialize-NotesScriptsDir -ScriptsDirectory $PSScriptRoot

$RepoRoot = Get-NotesRepoRoot
$BridgeDir = Join-Path $RepoRoot "bridge"
$BridgePidFile = Join-Path $BridgeDir ".bridge.pid"
$BridgePort = Get-PokydBridgePort -RepoRoot $RepoRoot
$env:BRIDGE_PORT = [string]$BridgePort

$bridgeProc = $null

function Stop-ListenersOnPort {
    param([int] $Port)
    try {
        $conns = Get-NetTCPConnection -LocalPort $Port -State Listen -ErrorAction SilentlyContinue
        if ($conns) {
            $owningPids = $conns | Select-Object -ExpandProperty OwningProcess -Unique
            foreach ($owningPid in $owningPids) {
                if (-not $owningPid) { continue }
                Write-Host "[gpt] Releasing TCP $Port - stopping pid $owningPid..."
                Stop-Process -Id $owningPid -Force -ErrorAction SilentlyContinue
            }
            Start-Sleep -Milliseconds 400
            return
        }
    } catch {
        # ignore and try netstat
    }

    $matchesLines = @(netstat -ano 2>$null | Select-String ":$Port\s.*LISTENING")
    foreach ($m in $matchesLines) {
        $parts = @($m.Line -split '\s+' | Where-Object { $_ -ne '' })
        if ($parts.Count -lt 1) { continue }
        $last = $parts[$parts.Count - 1]
        if ($last -match '^\d+$') {
            Write-Host "[gpt] Releasing TCP $Port (netstat) - stopping pid $last..."
            & taskkill.exe /PID $last /F 2>$null | Out-Null
        }
    }
    Start-Sleep -Milliseconds 400
}

function Invoke-GptCleanup {
    param([switch] $Silent)
    if ($bridgeProc -and -not $bridgeProc.HasExited) {
        if (-not $Silent) {
            Write-Host "[gpt] Stopping Node bridge (pid $($bridgeProc.Id))..."
        }
        Stop-Process -Id $bridgeProc.Id -Force -ErrorAction SilentlyContinue
        try { $bridgeProc.WaitForExit(5000) | Out-Null } catch { }
    }
    Stop-ListenersOnPort -Port $BridgePort
    Remove-Item -LiteralPath $BridgePidFile -Force -ErrorAction SilentlyContinue
}

$dbExit = 0
try {
    if (-not (Get-Command node -ErrorAction SilentlyContinue)) {
        throw "node not found. Install Node.js 18+."
    }

    if (-not (Test-Path -LiteralPath (Join-Path $BridgeDir ".env"))) {
        throw "Missing bridge/.env - copy bridge/.env.example and set OPENAI_API_KEY."
    }

    $hasOpenAiKey = $false
    Get-Content -LiteralPath (Join-Path $BridgeDir ".env") | ForEach-Object {
        if ($_ -match '^\s*OPENAI_API_KEY\s*=\s*(.+)$') {
            $val = $Matches[1].Trim().Trim('"')
            if ($val.Length -gt 0) { $hasOpenAiKey = $true }
        }
    }
    if (-not $hasOpenAiKey) {
        throw "bridge/.env must set OPENAI_API_KEY=... (non-empty)."
    }

    $wattHeader = Join-Path $RepoRoot "vendor\watt32-dos\inc\tcp.h"
    if (-not (Test-Path -LiteralPath $wattHeader)) {
        throw @"
Watt-32 bundle missing: vendor\watt32-dos\inc\tcp.h
Run scripts\bootstrap-watt32-docker.bat or scripts/bootstrap-watt32-docker.sh, or clone Watt-32 into vendor/watt32-dos.
"@
    }

    Write-Host "[gpt] Installing bridge npm dependencies (if needed)..."
    Push-Location $BridgeDir
    try {
        & npm install --silent
        if ($LASTEXITCODE -ne 0) { throw "npm install failed with exit $LASTEXITCODE" }
    } finally {
        Pop-Location
    }

    Write-Host "[gpt] Ensuring TCP port $BridgePort is free (previous bridge / crash)..."
    Stop-ListenersOnPort -Port $BridgePort

    $logOut = Join-Path $BridgeDir "pokyd-bridge.log"
    $logErr = Join-Path $BridgeDir "pokyd-bridge.err"
    Write-Host "[gpt] Starting OpenAI bridge on port $BridgePort..."
    $nodeExe = (Get-Command node).Source
    $bridgeProc = Start-Process -FilePath $nodeExe -ArgumentList @("server.js") `
        -WorkingDirectory $BridgeDir -PassThru -WindowStyle Hidden `
        -RedirectStandardOutput $logOut -RedirectStandardError $logErr

    Set-Content -LiteralPath $BridgePidFile -Value $bridgeProc.Id -Encoding ASCII

    Start-Sleep -Seconds 1
    if ($bridgeProc.HasExited) {
        Write-Host "[gpt] Bridge failed to start. Last lines of bridge logs:"
        if (Test-Path -LiteralPath $logOut) { Get-Content -LiteralPath $logOut -Tail 20 }
        if (Test-Path -LiteralPath $logErr) { Get-Content -LiteralPath $logErr -Tail 20 }
        throw "Bridge exited early."
    }

    Write-Host "[gpt] Bridge pid=$($bridgeProc.Id) (log: bridge/pokyd-bridge.log, pokyd-bridge.err)"
    $tnc = Test-NetConnection -ComputerName 127.0.0.1 -Port $BridgePort -WarningAction SilentlyContinue
    if (-not $tnc.TcpTestSucceeded) {
        throw "Bridge is not listening on TCP $BridgePort (check bridge/pokyd-bridge.err)."
    }
    Write-Host "[gpt] Building pokyd.exe with LLM (Watt-32)..."
    $llmHost = Resolve-PokydLlmHost -RepoRoot $RepoRoot
    $env:POKYD_LLM_HOST = $llmHost
    if ($llmHost -match '^([^:]+):(\d+)$') {
        $env:POKYD_LLM_IP = $Matches[1]
        $env:POKYD_LLM_PORT = $Matches[2]
    }
    $buildBat = Join-Path $RepoRoot "build.bat"
    & $buildBat
    if ($LASTEXITCODE -ne 0) { throw "build.bat failed with exit $LASTEXITCODE" }

    $exeBytes = [System.IO.File]::ReadAllBytes((Join-Path $RepoRoot "pokyd.exe"))
    $exeText = [System.Text.Encoding]::ASCII.GetString($exeBytes)
    if ($exeText.Contains("LLM_INIT: no Watt-32")) {
        throw "pokyd.exe was built without Watt-32 LLM support. Check build.bat output."
    }
    if (-not $exeText.Contains("LLM_INIT: host=")) {
        throw "pokyd.exe does not contain Watt-32 LLM code. Re-run scripts\bootstrap-watt32-docker.bat."
    }

    Write-Host "[gpt] Launching DOSBox-X with -llm $llmHost (DOS-safe argv)"

    $runArgs = @{
        EnableLlm = $true
        LlmHost   = $llmHost
    }
    if ($ExitAfterPokyd) { $runArgs.ExitAfterNotes = $true }
    if ($SkipIntro) { $runArgs.SkipIntro = $true }

    & (Join-Path $PSScriptRoot "run-dosbox.ps1") @runArgs
    $dbExit = $LASTEXITCODE
    if ($dbExit -eq 0) {
        Write-Host ""
        Write-Host '[gpt] Session finished - you closed DOSBox-X (or typed EXIT at the C:\> prompt).'
        Write-Host '[gpt] This is normal, not an error. The Node bridge is shut down too.'
    } else {
        Write-Host ('[gpt] DOSBox exited with code ' + $dbExit + '. Check DEBUG.LOG and bridge/pokyd-bridge.log.')
    }
} catch {
    Write-Host $_
    $dbExit = 1
} finally {
    Invoke-GptCleanup -Silent
}

exit $dbExit
