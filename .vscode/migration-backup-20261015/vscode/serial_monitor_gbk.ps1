param(
    [string]$Port = 'COM11',
    [int]$Baud = 115200,
    [switch]$Check
)

$ErrorActionPreference = 'Stop'

if ($Check) {
    $ports = [System.IO.Ports.SerialPort]::GetPortNames()
    Write-Output "Available ports: $($ports -join ', ')"
    Write-Output "Target: $Port @ $Baud baud (GBK decode)"
    exit 0
}

[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$gbk = [System.Text.Encoding]::GetEncoding(936)
$decoder = $gbk.GetDecoder()
$portObj = New-Object System.IO.Ports.SerialPort($Port, $Baud, [System.IO.Ports.Parity]::None, 8, [System.IO.Ports.StopBits]::One)
$portObj.ReadTimeout = 500

try {
    $portObj.Open()
    Write-Output "Connected to $Port at $Baud baud (GBK). Stop the task to close."
    $chars = New-Object char[] 4096
    while ($true) {
        if ($portObj.BytesToRead -gt 0) {
            $buf = New-Object byte[] $portObj.BytesToRead
            $read = $portObj.Read($buf, 0, $buf.Length)
            if ($read -gt 0) {
                $count = $decoder.GetChars($buf, 0, $read, $chars, 0, $false)
                if ($count -gt 0) {
                    [Console]::Write([string]::new($chars, 0, $count))
                }
            }
        }
        else {
            Start-Sleep -Milliseconds 50
        }
    }
}
finally {
    if ($portObj.IsOpen) {
        $portObj.Close()
    }
}
