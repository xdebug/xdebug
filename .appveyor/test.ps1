$dir = 'C:\projects\xdebug\'
if ($env:ARCH -eq 'x64') {
    $dir += 'x64\'
}
$dir += 'Release'
if ($env:TS -eq '1') {
    $dir += '_TS'
}

$xdebug_dll_opt = "-d zend_extension=$dir\php_xdebug.dll"

Set-Location "C:\projects\xdebug"
[void](New-Item "C:\tests_tmp" -ItemType 'directory')

$php = Get-Command 'php' | Select-Object -ExpandProperty 'Definition'
$dname = (Get-Item $php).Directory.FullName

$opts = '-n -d foo=yes'
if ($env:OPCACHE -eq 'yes') {
    $opts += " -d zend_extension=$dname\ext\php_opcache.dll -d opcache.enabled=1 -d opcache.enable_cli=1"
}
$opts += " $xdebug_dll_opt"
$env:TEST_PHP_ARGS = $opts

$env:TEST_PHP_EXECUTABLE = $php
& $php run-xdebug-tests.php -q --offline --show-diff --show-slow 1000 --set-timeout 120 -g FAIL,XFAIL,BORK,WARN,LEAK,SKIP --temp-source C:\tests_tmp --temp-target C:\tests_tmp
