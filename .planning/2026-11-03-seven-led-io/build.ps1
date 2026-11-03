$ErrorActionPreference = 'Stop'
$taskRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\..')).Path
$taskBuildDir = Join-Path $PSScriptRoot 'build'
$taskProject = Join-Path $taskRoot 'KEIL_PRJ\seven_led_validation.uvprojx'
$taskLog = Join-Path $taskBuildDir 'rebuild.log'
$taskUv4 = 'D:\Users\mym02\AppData\Local\Keil_v5\UV4\UV4.exe'
New-Item -ItemType Directory -Path $taskBuildDir -Force | Out-Null
$taskProjectText = Get-Content -LiteralPath (Join-Path $taskRoot 'KEIL_PRJ\MM32SPIN0230_Demo.uvprojx') -Raw
$taskProjectText = $taskProjectText.Replace('<OutputDirectory>.\Objects\</OutputDirectory>', '<OutputDirectory>..\.planning\2026-11-03-seven-led-io\build\Objects\</OutputDirectory>')
$taskProjectText = $taskProjectText.Replace('<ListingPath>.\Listings\</ListingPath>', '<ListingPath>..\.planning\2026-11-03-seven-led-io\build\Listings\</ListingPath>')
$taskProjectText = $taskProjectText.Replace('--output=@L.bin', '--output=..\.planning\2026-11-03-seven-led-io\build\SPIN0230_DEMO.bin')
[System.IO.File]::WriteAllText($taskProject, $taskProjectText, [System.Text.UTF8Encoding]::new($false))
$taskArguments = '-r "' + $taskProject + '" -t MM32SPIN0230 -o "' + $taskLog + '"'
$taskProcess = Start-Process -FilePath $taskUv4 -ArgumentList $taskArguments -WorkingDirectory (Join-Path $taskRoot 'KEIL_PRJ') -Wait -PassThru -WindowStyle Hidden
Write-Output "UV4 exit code: $($taskProcess.ExitCode)"
if(Test-Path -LiteralPath $taskLog) { Get-Content -LiteralPath $taskLog }
if($taskProcess.ExitCode -gt 1) { throw 'Keil clean rebuild failed.' }
$taskHex = Join-Path $taskBuildDir 'Objects\SPIN0230_DEMO.hex'
if(!(Test-Path -LiteralPath $taskHex)) { throw 'Missing firmware hex.' }
Get-Item -LiteralPath $taskHex | Select-Object FullName, Length, LastWriteTime
