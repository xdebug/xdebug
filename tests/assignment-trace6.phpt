--TEST--
Test for tracing assignments in user-readable function traces
--SKIPIF--
<?php if (extension_loaded('zend opcache')) echo "skip opcache should not be loaded\n"; ?>
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
	static public $foo;
	static private $var;
	static public $var2;
	static public $bar;
	static function test()
	{
		self::$foo = array();
		self::$foo[] = 42;
		self::$var = 'var';
		self::$var2 = self::$var;
		$id = 42;
		self::$foo[$id] = 44;

		self::$bar['test'] = array();
		$id = 'test';
		self::$bar[$id][] = 55;
	}
}
foo::test();

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d]
                           => $tf = '%s' %sassignment-trace6.php:2
%w%f %w%d     -> foo::test() %sassignment-trace6.php:23
                             => self::foo = array () %sassignment-trace6.php:11
                             => self::foo[] = 42 %sassignment-trace6.php:12
                             => self::var = 'var' %sassignment-trace6.php:13
                             => self::var2 = 'var' %sassignment-trace6.php:14
                             => $id = 42 %sassignment-trace6.php:15
                             => self::foo[42] = 44 %sassignment-trace6.php:16
                             => self::bar['test'] = array () %sassignment-trace6.php:18
                             => $id = 'test' %sassignment-trace6.php:19
                             => self::bar['test'][] = 55 %sassignment-trace6.php:20
%w%f %w%d     -> xdebug_stop_trace() %sassignment-trace6.php:25
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d]
