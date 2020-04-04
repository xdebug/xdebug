--TEST--
Test for bug #419: make use of P_tmpdir if defined instead of hardcoded '/tmp'
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('linux');
?>
--FILE--
<?php
echo ini_get( 'xdebug.output_dir' ), "\n";
?>
--EXPECT--
/tmp
