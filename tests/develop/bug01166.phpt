--TEST--
Test for bug #1166: Using $this in __debugInfo() causes infinite recursion
--INI--
xdebug.mode=develop
xdebug.trace_format=0
xdebug.dump_globals=0
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
Warning: Uncaught%sThe object is in an invalid state as the parent constructor was not called%sin %sbug01166.php:14
Stack trace:
#0 [internal function]: IteratorIterator->rewind()
#1 %sbug01166.php(%d): iterator_count(Object(Foo))
#2 [internal function]: Foo->__debugInfo()
#3 %sbug01166.php(%d): var_dump(Object(Foo))
#4 {main}
  thrown in %sbug01166.php on line %d

Call Stack:
%w%f %w%d   1. {main}() %sbug01166.php:0
%w%f %w%d   2. var_dump(%s) %sbug01166.php:%d


Fatal error: __debuginfo() must return an array in %sbug01166.php on line %d

Call Stack:
%w%f %w%d   1. {main}() %sbug01166.php:0
%w%f %w%d   2. var_dump(%s) %sbug01166.php:%d
