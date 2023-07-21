--TEST--
Test for tracing multi-dimensional property assignments in user-readable function traces
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.trace_format=0
xdebug.collect_return=0
xdebug.collect_assignments=1
xdebug.var_display_max_depth=9
--FILE--
<?php
require_once 'capture-trace.inc';

class testClass
{
	public $a;
	static public $b;

	function __construct( $obj )
	{
		$obj->a = new StdClass;
		$obj->a->bar = 52;
		$obj->a->foo = new StdClass;
		$obj->a->foo->bar = 52;

		$this->a = new StdClass;
		$this->a->bar = 52;
		$this->a->foo = new StdClass;
		$this->a->foo->bar = 52;

		self::$b = new StdClass;
		self::$b->bar = 52;
		self::$b->foo = new StdClass;
		self::$b->foo->bar = 52;

		static::$b = new StdClass;
		static::$b->bar = 52;
		static::$b->foo = new StdClass;
		static::$b->foo->bar = 52;
	}
}

$a = new testClass( new StdClass );

xdebug_stop_trace();
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
                             => $tf = '%sxt%S' %s:%d
%w%f %w%d     -> testClass->__construct($obj = class stdClass {  }) %sassignment-trace-008.php:33
                             => $obj->a = class stdClass {  } %sassignment-trace-008.php:11
                             => $obj->a->bar = 52 %sassignment-trace-008.php:12
                             => $obj->a->foo = class stdClass {  } %sassignment-trace-008.php:13
                             => $obj->a->foo->bar = 52 %sassignment-trace-008.php:14
                             => $this->a = class stdClass {  } %sassignment-trace-008.php:16
                             => $this->a->bar = 52 %sassignment-trace-008.php:17
                             => $this->a->foo = class stdClass {  } %sassignment-trace-008.php:18
                             => $this->a->foo->bar = 52 %sassignment-trace-008.php:19
                             => self::b = class stdClass {  } %sassignment-trace-008.php:21
                             => self::b->bar = 52 %sassignment-trace-008.php:22
                             => self::b->foo = class stdClass {  } %sassignment-trace-008.php:23
                             => self::b->foo->bar = 52 %sassignment-trace-008.php:24
                             => self::b = class stdClass {  } %sassignment-trace-008.php:26
                             => self::b->bar = 52 %sassignment-trace-008.php:27
                             => self::b->foo = class stdClass {  } %sassignment-trace-008.php:28
                             => self::b->foo->bar = 52 %sassignment-trace-008.php:29
                           => $a = class testClass { public $a = class stdClass { public $bar = 52; public $foo = class stdClass { public $bar = 52 } } } %sassignment-trace-008.php:33
%w%f %w%d     -> xdebug_stop_trace() %sassignment-trace-008.php:35
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
