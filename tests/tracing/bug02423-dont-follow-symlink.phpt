--TEST--
Test for bug #2423: Don't follow symlinks with file creation
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('!win');
?>
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.trace_format=0
xdebug.use_compression=0
--FILE--
<?php
require_once __DIR__ . '/../utils.inc';

$tmpBaseName = getTmpFile('bug2423-' . uniqid('', true));
$tmpTextName = $tmpBaseName . '.txt';
$tmpTraceName = $tmpBaseName . '.xt';

echo $tmpTextName, "\n";
echo $tmpTraceName, "\n";

file_put_contents($tmpTextName, "This is original text");
symlink($tmpTextName, $tmpTraceName);

$createdFileName = xdebug_start_trace($tmpBaseName, XDEBUG_TRACE_COMPUTERIZED);
xdebug_stop_trace();
echo $createdFileName, "\n";

echo file_get_contents($tmpTextName), "\n";

unlink($createdFileName);
unlink($tmpTextName);
unlink($tmpTraceName);
?>
--EXPECTF--
%sbug2423-%s.%d.txt
%sbug2423-%s.%d.xt
%sbug2423-%s.%d.%s.xt
This is original text
