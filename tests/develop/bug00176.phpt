--TEST--
Test for bug #176: Segfault using SplTempFileObject
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('ext SPL');
?>
--INI--
xdebug.mode=develop
xdebug.trace_options=0
xdebug.collect_return=1
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.dump_globals=0
xdebug.trace_format=0
--FILE--
<?php
try {
$crap = new SplTempFileObject(0);
$crap->fwrite('give me a crash');
}
catch ( Exception $e )
{
}
echo "DONE\n";
?>
--EXPECT--
DONE
