--TEST--
Test for bug #756: Tracing doesn't always understand the variables and shows IS_VAR
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.trace_options=0
xdebug.trace_output_name=trace.%c
xdebug.collect_return=1
xdebug.collect_assignments=1
xdebug.dump_globals=0
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

require_once 'capture-trace.inc';
$trace_file = xdebug_get_tracefile_name();

foo::bar();
$f = new foo;
$f->__construct();

xdebug_stop_trace();
echo "DONE\n";
?>
--EXPECTF--
DONE
TRACE START [%d-%d-%d %d:%d:%d.%d]
                             => $tf = '%sxdt%s.%s.xt%S' %s:%d
%w%f %w%d     -> xdebug_get_tracefile_name() %sbug00756.php:19
%w%f %w%d      >=> '%sxdt%s.%s.xt%S'
                           => $trace_file = '%sxdt%s.%s.xt%S' %sbug00756.php:19
%w%f %w%d     -> foo::bar() %sbug00756.php:21
                             => %r(\+\+self::bar|self::bar\+\+)%r %sbug00756.php:9
%w%f %w%d     -> foo->__construct() %sbug00756.php:22
                             => %r(\$this->foo\+\+|\+\+\$this->foo)%r %sbug00756.php:14
                           => $f = class foo { public $foo = 1 } %sbug00756.php:22
%w%f %w%d     -> foo->__construct() %sbug00756.php:23
                             => %r(\$this->foo\+\+|\+\+\$this->foo)%r %sbug00756.php:14
%w%f %w%d     -> xdebug_stop_trace() %sbug00756.php:25
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
