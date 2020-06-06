--TEST--
GC Stats: Class with garbage
--INI--
xdebug.mode=gcstats
xdebug.start_with_request=yes
zend.enable_gc=1
--FILE--
<?php

function foo()
{
	bar();
}

function bar()
{
	for ($i = 0; $i < 20000; $i++)
	{
		$a = new stdClass();
		$b = new stdClass();
		$b->a = $a;
		$a->b = $b;
		unset($a, $b);
	}
}

class Garbage
{
	public function produce()
	{
		$foo = new stdClass();

		for ($i = 0; $i < 20000; $i++)
		{
			$a = new stdClass();
			$b = new stdClass();
			$b->foo = $foo;
			$b->a = $a;
			$a->b = $b;
			unset($a, $b);
		}

		unset($foo);
		gc_collect_cycles();
	}
}

foo();
(new Garbage())->produce();

$data = file_get_contents(xdebug_get_gcstats_filename());
xdebug_stop_gcstats();
unlink(xdebug_get_gcstats_filename());

var_dump(substr_count($data, "bar") >= 3);
var_dump(substr_count($data, "Garbage::produce") >= 4);
var_dump(substr_count($data, "gc_collect_cycles") == 1);
?>
--EXPECTF--
bool(true)
bool(true)
bool(true)
