--TEST--
GC Stats: Disabling Zend GC during the script
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 7.3');
?>
--INI--
xdebug.mode=gcstats
xdebug.start_with_request=yes
zend.enable_gc=1
report_memleaks=0
xdebug.log=
xdebug.log_level=10
--FILE--
<?php

function foo()
{
	bar();
}

function bar()
{
	for ($i = 0; $i < 20000; $i++) {
		$a = new stdClass();
		$b = new stdClass();
		$b->a = $a;
		$a->b = $b;
		unset($a, $b);
	}
}

foo();

gc_disable();

xdebug_stop_gcstats();

$lines = file(xdebug_get_gcstats_filename());

unlink(xdebug_get_gcstats_filename());

var_dump(count($lines) >= 6);
?>
--EXPECTF--
Xdebug: [GC Stats] PHP's Garbage Collection is disabled at the end of the script
bool(true)
