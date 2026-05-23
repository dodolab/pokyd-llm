# Shared LLM / bridge environment for PowerShell scripts (dot-source; do not run directly).
#
# Cross-platform names (same as scripts/pokyd-llm-env.sh):
#   BRIDGE_PORT, POKYD_LLM_HOST, POKYD_LLM_IP, POKYD_LLM_PORT, POKYD_DOSBOX_CPU_CORE

function Get-PokydBridgePortDefault {
    return 8765
}

function Get-PokydBridgePort {
    param(
        [Parameter(Mandatory)][string] $RepoRoot
    )

    $port = $null
    if ($env:BRIDGE_PORT -match '^\s*(\d+)\s*$') {
        $p = [int]$Matches[1]
        if ($p -gt 0) { $port = $p }
    }

    $envPath = Join-Path $RepoRoot "bridge\.env"
    if (-not $port -and (Test-Path -LiteralPath $envPath)) {
        Get-Content -LiteralPath $envPath | ForEach-Object {
            if ($_ -match '^\s*BRIDGE_PORT\s*=\s*([^#]+)') {
                $v = $Matches[1].Trim().Trim('"').Trim()
                $parsed = 0
                if ([int]::TryParse($v, [ref]$parsed) -and $parsed -gt 0) {
                    $port = $parsed
                }
            }
        }
    }

    if (-not $port) {
        $port = Get-PokydBridgePortDefault
    }
    return $port
}

function Resolve-PokydLlmHost {
    param(
        [Parameter(Mandatory)][string] $RepoRoot
    )

    if ($env:POKYD_LLM_HOST -and $env:POKYD_LLM_HOST.Trim().Length -gt 0) {
        return $env:POKYD_LLM_HOST.Trim()
    }

    $ip = if ($env:POKYD_LLM_IP -and $env:POKYD_LLM_IP.Trim().Length -gt 0) {
        $env:POKYD_LLM_IP.Trim()
    } else {
        # Slirp NAT gateway (see DOSBox-X wiki ipv4_host default).
        '10.0.2.2'
    }

    $port = $null
    if ($env:POKYD_LLM_PORT -match '^\s*(\d+)\s*$') {
        $p = [int]$Matches[1]
        if ($p -gt 0) { $port = $p }
    }
    if (-not $port) {
        $port = Get-PokydBridgePort -RepoRoot $RepoRoot
    }

    return "${ip}:$port"
}

function Test-PokydLlmConfigured {
    return (
        ($env:POKYD_LLM_HOST -and $env:POKYD_LLM_HOST.Trim().Length -gt 0) -or
        ($env:POKYD_LLM_IP -and $env:POKYD_LLM_IP.Trim().Length -gt 0) -or
        ($env:POKYD_LLM_PORT -match '^\s*\d+\s*$')
    )
}
