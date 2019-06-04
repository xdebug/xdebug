--TEST--
Test for bug #419: make use of P_tmpdir if defined instead of hardcoded '/tmp'
--SKIPIF--
<?php
require 'tests/utils.inc';
check_reqs('sunos');
?>
--FILE--
<?php
echo ini_get( 'xdebug.trace_output_dir' ), "\n";
echo ini_get( 'xdebug.profiler_output_dir' ), "\n";
?>
--EXPECT--
/var/tmp/
/var/tmp/
