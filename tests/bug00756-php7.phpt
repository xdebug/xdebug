--TEST--
Test for bug #756: Tracing doesn't always understand the variables and shows IS_VAR (>= PHP 7.0)
--SKIPIF--
<?php if (!version_compare(phpversion(), "7.0", '>=')) echo "skip >= PHP 7.0 needed\n"; ?>
<?php if (extension_loaded('zend opcache')) echo "skip opcache should not be loaded\n"; ?>
--INI--
xdebug.auto_trace=0
xdebug.trace_options=0
xdebug.trace_output_dir=/tmp
xdebug.trace_output_name=trace.%c
xdebug.collect_return=1
xdebug.collect_params=3
xdebug.collect_assignments=1
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.dump_globals=0
xdebug.show_mem_delta=0
xdebug.trace_format=0
--FILE--
<?php
class foo
{
	static $bar;
	public $foo;

	static function bar()
	{
		self::$bar++;
	}

	function __construct()
	{
		$this->foo++;
	}
}

$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));
$trace_file = xdebug_get_tracefile_name();

foo::bar();
$f = new foo;
$f->__construct();

xdebug_stop_trace();
echo file_get_contents($trace_file);
unlink($trace_file);
echo "DONE\n";
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d]
                           => $tf = '%sxdt%s.%s.xt' %sbug00756-php7.php:18
%w%f %w%d     -> xdebug_get_tracefile_name() %sbug00756-php7.php:19
%w%f %w%d      >=> '%sxdt%s.%s.xt'
                           => $trace_file = '%sxdt%s.%s.xt' %sbug00756-php7.php:19
%w%f %w%d     -> foo::bar() %sbug00756-php7.php:21
                             => self::bar++ %sbug00756-php7.php:9
%w%f %w%d     -> foo->__construct() %sbug00756-php7.php:22
                             => $this->foo++ %sbug00756-php7.php:14
                           => $f = class foo { public $foo = 1 } %sbug00756-php7.php:22
%w%f %w%d     -> foo->__construct() %sbug00756-php7.php:23
                             => $this->foo++ %sbug00756-php7.php:14
%w%f %w%d     -> xdebug_stop_trace() %sbug00756-php7.php:25
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d]

DONE
