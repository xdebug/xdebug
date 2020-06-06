--TEST--
GC Stats: Stats about trigger garbage collection automatically
--INI--
xdebug.mode=gcstats
xdebug.start_with_request=yes
zend.enable_gc=1
--FILE--
<?php

gc_enable();

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

$lines = file(xdebug_get_gcstats_filename());
xdebug_stop_gcstats();
unlink(xdebug_get_gcstats_filename());

var_dump(count($lines) >= 6);
?>
--EXPECTF--
bool(true)
