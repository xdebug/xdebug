--TEST--
Function Monitor: Method function name
--FILE--
<?php
class Foo
{
	static function doStuff()
	{
	}

	public function doMethodStuff()
	{
	}
}

xdebug_start_function_monitor( [ 'Foo::doStuff', 'Foo->doMethodStuff' ] );

Foo::doStuff();

$f = new Foo;
$f->doMethodStuff();

var_dump(xdebug_get_monitored_functions());
xdebug_stop_function_monitor();
?>
--EXPECT--
array(2) {
  [0]=>
  string(12) "Foo::doStuff"
  [1]=>
  string(18) "Foo->doMethodStuff"
}
