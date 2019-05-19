--TEST--
Test for tracing new PHP 7.4 assignment opcodes (< PHP 7.4 && opcache)
--SKIPIF--
<?php
require 'utils.inc';
if ( ! ( runtime_version( '7.4', '<' ) && opcache_active() ) ) {
	echo "skip < PHP 7.4 && opcache active needed\n";
};
?>
--INI--
xdebug.default_enable=1
xdebug.profiler_enable=0
xdebug.auto_trace=0
xdebug.trace_format=0
xdebug.collect_vars=1
xdebug.collect_params=3
xdebug.collect_return=0
xdebug.collect_assignments=1
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

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
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d]
                           => $tf = '%s' %sbug01599-02.php:2
%w%f %w%d     -> foo::test() %sbug01599-02.php:22
                             => self::var1 = 42 %sbug01599-02.php:12
                             => self::var2 = 42 %sbug01599-02.php:13
                             => self::var3 &= 42 %sbug01599-02.php:14
                             => self::var3 =& self::var2 %sbug01599-02.php:15
                             => ++self::var4 %sbug01599-02.php:16
                             => --self::var4 %sbug01599-02.php:17
                             => ++self::var4 %sbug01599-02.php:18
                             => --self::var4 %sbug01599-02.php:19
%w%f %w%d     -> xdebug_stop_trace() %sbug01599-02.php:24
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d]
