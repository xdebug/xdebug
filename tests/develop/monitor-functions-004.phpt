--TEST--
Function Monitor: Method function name
--INI--
xdebug.mode=develop
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
--EXPECTF--
%smonitor-functions-004.php:20:
array(2) {
  [0] =>
  array(3) {
    'function' =>
    string(12) "Foo::doStuff"
    'filename' =>
    string(%d) "%smonitor-functions-004.php"
    'lineno' =>
    int(15)
  }
  [1] =>
  array(3) {
    'function' =>
    string(18) "Foo->doMethodStuff"
    'filename' =>
    string(%d) "%smonitor-functions-004.php"
    'lineno' =>
    int(18)
  }
}
