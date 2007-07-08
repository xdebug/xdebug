--TEST--
Test for bug #176: Segfault using SplTempFileObject
--SKIPIF--
<?php if (!extension_loaded("xdebug")) print "skip"; ?>
<?php if (!extension_loaded("SPL")) print "skip No SPL available"; ?>
<?php if(version_compare(zend_version(), "2.2.0-dev", '<')) echo "skip Zend Engine 2.2 needed\n"; ?>
--INI--
xdebug.default_enable=1
xdebug.auto_trace=0
xdebug.trace_options=0
xdebug.trace_output_dir=/tmp
xdebug.collect_return=1
xdebug.collect_params=1
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.dump_globals=0
xdebug.show_mem_delta=0
xdebug.trace_format=0
--FILE--
<?php
try {
$crap = new SplTempFileObject('foo');
$crap->fwrite('give me a crash');
}
catch ( Exception $e )
{
}
echo "DONE\n";
?>
--EXPECT--
DONE
