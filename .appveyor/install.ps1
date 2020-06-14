$ErrorActionPreference = "Stop"

if (-not (Test-Path 'C:\build-cache')) {
    [void](New-Item 'C:\build-cache' -ItemType 'directory')
}
$bname = "php-sdk-$env:BIN_SDK_VER.zip"
if (-not (Test-Path "C:\build-cache\$bname")) {
    Invoke-WebRequest "https://github.com/OSTC/php-sdk-binary-tools/archive/$bname" -OutFile "C:\build-cache\$bname"
}
$dname0 = "php-sdk-binary-tools-php-sdk-$env:BIN_SDK_VER"
$dname1 = "php-sdk-$env:BIN_SDK_VER"
if (-not (Test-Path "C:\build-cache\$dname1")) {
    Expand-Archive "C:\build-cache\$bname" 'C:\build-cache'
    Move-Item "C:\build-cache\$dname0" "C:\build-cache\$dname1"
}

$releases = Invoke-WebRequest https://windows.php.net/downloads/releases/releases.json | ConvertFrom-Json
$phpversion = $releases.$env:PHP_VER.version

$ts_part = ''
if ($env:TS -eq '0') {
    $ts_part += '-nts'
}
$bname = "php-devel-pack-$phpversion$ts_part-Win32-$env:VC-$env:ARCH.zip"
if (-not (Test-Path "C:\build-cache\$bname")) {
    try {
        Invoke-WebRequest "https://windows.php.net/downloads/releases/archives/$bname" -OutFile "C:\build-cache\$bname"
    } catch [System.Net.WebException] { 
        Invoke-WebRequest "https://windows.php.net/downloads/releases/$bname" -OutFile "C:\build-cache\$bname"
    }
}
$dname0 = "php-$phpversion-devel-$env:VC-$env:ARCH"
$dname1 = "php-$phpversion$ts_part-devel-$env:VC-$env:ARCH"
if (-not (Test-Path "C:\build-cache\$dname1")) {
    Expand-Archive "C:\build-cache\$bname" 'C:\build-cache'
    if ($dname0 -ne $dname1) {
        Move-Item "C:\build-cache\$dname0" "C:\build-cache\$dname1"
    }
}
$env:PATH = "C:\build-cache\$dname1;$env:PATH"

$bname = "php-$phpversion$ts_part-Win32-$env:VC-$env:ARCH.zip"
if (-not (Test-Path "C:\build-cache\$bname")) {
    try {
        Invoke-WebRequest "https://windows.php.net/downloads/releases/archives/$bname" -OutFile "C:\build-cache\$bname"
    } catch [System.Net.WebException] { 
        Invoke-WebRequest "https://windows.php.net/downloads/releases/$bname" -OutFile "C:\build-cache\$bname"
    }
}
$dname = "php-$phpversion$ts_part-$env:VC-$env:ARCH"
if (-not (Test-Path "C:\build-cache\$dname")) {
    Expand-Archive "C:\build-cache\$bname" "C:\build-cache\$dname"
}
$env:PATH = "c:\build-cache\$dname;$env:PATH"
