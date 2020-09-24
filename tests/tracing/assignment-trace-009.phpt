--TEST--
Test for tracing mixed array element and property assignments in user-readable function traces
--INI--
xdebug.mode=trace
xdebug.start_with_request=0
xdebug.trace_format=0
xdebug.collect_return=0
xdebug.collect_assignments=1
xdebug.var_display_max_depth=9
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

class testClass
{
	public $a;
	static public $b;

	function __construct( $obj )
	{
		$obj->a = array();
		$obj->a['bar'] = 52;
		$obj->a['foo'] = new StdClass;
		$obj->a['foo']->bar = 52;

		$this->a = array();
		$this->a['bar'] = 52;
		$this->a['foo'] = new StdClass;
		$this->a['foo']->bar = 52;

		self::$b = array();
		self::$b['bar'] = array();
		self::$b['foo'] = new StdClass;
		self::$b['foo']->bar = 52;

		static::$b = array();
		static::$b['bar'] = array();
		static::$b['foo'] = new StdClass;
		static::$b['foo']->bar = 52;
	}
}

$a = new testClass( new StdClass );

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
                           => $tf = '%s.xt' %sassignment-trace-009.php:2
%w%f %w%d     -> testClass->__construct($obj = class stdClass {  }) %sassignment-trace-009.php:33
                             => $obj->a = [] %sassignment-trace-009.php:11
                             => $obj->a['bar'] = 52 %sassignment-trace-009.php:12
                             => $obj->a['foo'] = class stdClass {  } %sassignment-trace-009.php:13
                             => $obj->a['foo']->bar = 52 %sassignment-trace-009.php:14
                             => $this->a = [] %sassignment-trace-009.php:16
                             => $this->a['bar'] = 52 %sassignment-trace-009.php:17
                             => $this->a['foo'] = class stdClass {  } %sassignment-trace-009.php:18
                             => $this->a['foo']->bar = 52 %sassignment-trace-009.php:19
                             => self::b = [] %sassignment-trace-009.php:21
                             => self::b['bar'] = [] %sassignment-trace-009.php:22
                             => self::b['foo'] = class stdClass {  } %sassignment-trace-009.php:23
                             => self::b['foo']->bar = 52 %sassignment-trace-009.php:24
                             => self::b = [] %sassignment-trace-009.php:26
                             => self::b['bar'] = [] %sassignment-trace-009.php:27
                             => self::b['foo'] = class stdClass {  } %sassignment-trace-009.php:28
                             => self::b['foo']->bar = 52 %sassignment-trace-009.php:29
                           => $a = class testClass { public $a = ['bar' => 52, 'foo' => class stdClass { public $bar = 52 }] } %sassignment-trace-009.php:33
%w%f %w%d     -> xdebug_stop_trace() %sassignment-trace-009.php:35
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
