param(
    [switch]$Check
)

$ErrorActionPreference = 'Stop'

$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path

$cliCandidates = @(
    (Get-Item Env:STM32_PROGRAMMER -ErrorAction SilentlyContinue).Value,
    'D:\STUDY\Cube CLT\STM32CubeCLT_1.18.0\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe',
    'C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe'
) | Where-Object { $_ -and (Test-Path $_) } | Select-Object -First 1

if (-not $cliCandidates) {
    Write-Error 'STM32_Programmer_CLI.exe was not found. Install STM32CubeProgrammer or set STM32_PROGRAMMER to the CLI path.'
}

$hex = Join-Path $root 'Output\atk_f429.hex'
if (-not (Test-Path $hex)) {
    Write-Error "Hex file not found: $hex. Run the build task first."
}

if ($Check) {
    Write-Output "Programmer CLI: $cliCandidates"
    Write-Output "Hex file:       $hex"
    exit 0
}

& $cliCandidates -c 'port=SWD mode=UR' -w $hex -v -rst
exit $LASTEXITCODE
