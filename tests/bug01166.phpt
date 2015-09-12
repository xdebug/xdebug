--TEST--
Test for bug #1166: Using $this in __debugInfo() causes infinite recursion
--SKIPIF--
<?php if (!version_compare(phpversion(), "5.6", '>=')) echo "skip >= PHP 5.6 needed\n"; ?>
--INI--
xdebug.default_enable=1
xdebug.profiler_enable=0
xdebug.auto_trace=0
xdebug.trace_format=0
xdebug.dump_globals=0
xdebug.show_mem_delta=0
xdebug.collect_vars=0
xdebug.collect_params=3
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.force_error_reporting=0
--FILE--
<?php
class Foo extends IteratorIterator
{
	function __construct( $items )
	{
		if ( is_array( $items ) )
		{
			$items = new ArrayIterator( $items );
		}
	}
	
	function __debugInfo()
	{
		return [ "count" => iterator_count( $this ) ];
	}
}

$a = new Foo( [ 1,2,3 ] );
var_dump( $a );
?>
--EXPECTF--
Warning: Uncaught%sLogicException%sThe object is in an invalid state as the parent constructor was not called%sin %sbug01166.php:14
Stack trace:
#0 [internal function]: IteratorIterator->rewind()
#1 %sbug01166.php(%d): iterator_count(Object(Foo))
#2 [internal function]: Foo->__debugInfo()
#3 %sbug01166.php(%d): var_dump(Object(Foo))
#4 {main}
  thrown in %sbug01166.php on line %d

Call Stack:
    %f     %d   1. {main}() %sbug01166.php:0
    %f     %d   2. var_dump(class Foo {  }) %sbug01166.php:%d


Fatal error: __debuginfo() must return an array in %sbug01166.php on line %d

Call Stack:
    %f     %d   1. {main}() %sbug01166.php:0
    %f     %d   2. var_dump(class Foo {  }) %sbug01166.php:%d
