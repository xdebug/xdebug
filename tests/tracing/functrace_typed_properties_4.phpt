--TEST--
Test for function traces with typed properties (4)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 7.4');
?>
--INI--
xdebug.default_enable=1
xdebug.profiler_enable=0
xdebug.auto_trace=0
xdebug.trace_format=0
xdebug.dump_globals=0
xdebug.show_mem_delta=0
xdebug.collect_vars=0
xdebug.collect_params=4
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.force_error_reporting=0
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

class foo {
	public $v = M_PI;
	public $w;
	private string $x;
	protected int $y = 42;
	public ?Fibble $z;
	public \DateTime $a;
}

function test($value) {}

$f = new foo;
$f->a = new \DateTime;

test($f);
test(new class{public string $x;});

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d]
%w%f %w%d     -> DateTime->__construct() %sfunctrace_typed_properties_4.php:16
%w%f %w%d     -> test($value = class foo { public $v = 3.1415926535898; public $w = NULL; private string $x = *uninitialized*; protected int $y = 42; public ?Fibble $z = *uninitialized*; public DateTime $a = class DateTime { public $date = '%s'; public $timezone_type = 3; public $timezone = 'UTC' } }) %sfunctrace_typed_properties_4.php:18
%w%f %w%d     -> test($value = class class@anonymous { public string $x = *uninitialized* }) %sfunctrace_typed_properties_4.php:19
%w%f %w%d     -> xdebug_stop_trace() %sfunctrace_typed_properties_4.php:21
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d]
