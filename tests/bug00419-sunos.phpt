--TEST--
Test for bug #419: make use of P_tmpdir if defined instead of hardcoded '/tmp'
--SKIPIF--
<?php if (php_uname('s') != 'SunOS') echo "skip Solaris/OpenSolaris/OpenIndiana needed\n"; ?>
--FILE--
<?php
echo ini_get( 'xdebug.trace_output_dir' ), "\n";
echo ini_get( 'xdebug.profiler_output_dir' ), "\n";
?>
--EXPECT--
/var/tmp/
/var/tmp/
