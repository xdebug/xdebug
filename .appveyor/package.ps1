$ErrorActionPreference = "Stop"

$ts_part = ''
if ($env:TS -eq '0') {
    $ts_part += '-nts'
}
$arch_part = ''
if ($env:ARCH -eq 'x64') {
    $arch_part += '-x86_64'
}
if ($env:APPVEYOR_REPO_TAG -eq "true") {
    $bname = "php_xdebug-$env:APPVEYOR_REPO_TAG_NAME-" + "$env:PHP_VER-$env:VC$ts_part$arch_part"
} else {
    $bname = 'php_xdebug-' + $env:APPVEYOR_REPO_COMMIT.substring(0, 8) + "-$env:PHP_VER-$env:VC$ts_part$arch_part"
}
$zip_bname = "$bname.zip"
$dll_bname = "$bname.dll"

$dir = 'C:\projects\xdebug\'
if ($env:ARCH -eq 'x64') {
    $dir += 'x64\'
}
$dir += 'Release'
if ($env:TS -eq '1') {
    $dir += '_TS'
}

Copy-Item "$dir\php_xdebug.dll" "$dir\$dll_bname"
Compress-Archive @("$dir\$dll_bname", "$dir\php_xdebug.pdb", 'C:\projects\xdebug\LICENSE') "C:\$zip_bname"
Push-AppveyorArtifact "C:\$zip_bname"
