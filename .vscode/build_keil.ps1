param(
    [switch]$Rebuild
)

$ErrorActionPreference = 'Stop'

$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path

$uv4Candidates = @(
    (Get-Item Env:KEIL_UV4 -ErrorAction SilentlyContinue).Value,
    'D:\STUDY\Keil5\Core\UV4\UV4.exe',
    'C:\Keil_v5\UV4\UV4.exe',
    'D:\Keil_v5\UV4\UV4.exe',
    (Join-Path $env:ProgramFiles 'Keil_v5\UV4\UV4.exe'),
    (Join-Path ${env:ProgramFiles(x86)} 'Keil_v5\UV4\UV4.exe')
) | Where-Object { $_ -and (Test-Path $_) } | Select-Object -First 1

if (-not $uv4Candidates) {
    Write-Error 'Keil UV4.exe was not found. Install Keil MDK or set KEIL_UV4 to UV4.exe.'
}

$project = Join-Path $root 'Projects\MDK-ARM\atk_f429.uvprojx'
$log = Join-Path $root 'Output\vscode_build_log.txt'

if (Test-Path $log) {
    Remove-Item $log -Force
}

$mode = if ($Rebuild) { '-r' } else { '-b' }
$argList = @('-j0', $mode, "`"$project`"", '-o', "`"$log`"")

$proc = Start-Process -FilePath $uv4Candidates -ArgumentList $argList -Wait -PassThru -WindowStyle Hidden
Start-Sleep -Milliseconds 300

if (Test-Path $log) {
    $mdkDir = Join-Path $root 'Projects\MDK-ARM'
    $lines = Get-Content $log -Encoding Default
    foreach ($line in $lines) {
        if ($line -match '^([^\(]+)\((\d+)\):\s*(error|warning):') {
            $relative = $Matches[1].Trim()
            $absolute = [System.IO.Path]::GetFullPath((Join-Path $mdkDir $relative))
            $line = $line.Replace($relative, $absolute)
        }
        Write-Output $line
    }
}

exit $proc.ExitCode
