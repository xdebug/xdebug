--TEST--
Test for tracing new PHP 7.4 assignment opcodes
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.trace_format=0
xdebug.collect_return=0
xdebug.collect_assignments=1
--FILE--
<?php
require_once 'capture-trace.inc';

class foo {
	static public $var1;
	static public $var2;
	static public $var3;
	static public $var4 = 0;

	static function test()
	{
		self::$var1 = 42;
		self::$var2 = self::$var1;
		self::$var3 &= self::$var2;
		self::$var3 =& self::$var2;
		self::$var4++;
		self::$var4--;
		++self::$var4;
		--self::$var4;
	}
}
foo::test();

xdebug_stop_trace();
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
                             => $tf = '%sxt%S' %s:%d
%w%f %w%d     -> foo::test() %sbug01599-001.php:22
                             => self::var1 = 42 %sbug01599-001.php:12
                             => self::var2 = 42 %sbug01599-001.php:13
                             => self::var3 &= 42 %sbug01599-001.php:14
                             => self::var3 =& self::var2 %sbug01599-001.php:15
                             => ++self::var4 %sbug01599-001.php:16
                             => --self::var4 %sbug01599-001.php:17
                             => ++self::var4 %sbug01599-001.php:18
                             => --self::var4 %sbug01599-001.php:19
%w%f %w%d     -> xdebug_stop_trace() %sbug01599-001.php:24
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
