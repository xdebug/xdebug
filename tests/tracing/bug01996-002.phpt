--TEST--
Test for bug #1996: Show wrapped callable for first class callables in traces (PHP >= 8.1)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.1');
?>
--INI--
xdebug.mode=trace
xdebug.start_with_request=0
xdebug.collect_return=0
xdebug.collect_assignments=1
xdebug.trace_format=0
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));


$closure = substr(...);


function user_defined($a, $b)
{
	return substr($a, $b);
}
$closure = user_defined(...);


$closure = DateTimeImmutable::createFromFormat(...);


$dateTime = new DateTimeImmutable("2021-07-22");
$closure = $dateTime->format(...);


xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w             => $tf = '%s' %sbug01996-002.php:2
%w             => $closure = class Closure { virtual $closure = "substr", public $parameter = ['$string' => '<required>', '$offset' => '<required>', '$length' => '<optional>'] } %sbug01996-002.php:5
%w             => $closure = class Closure { virtual $closure = "user_defined", public $parameter = ['$a' => '<required>', '$b' => '<required>'] } %sbug01996-002.php:12
%w             => $closure = class Closure { virtual $closure = "DateTimeImmutable::createFromFormat", public $parameter = ['$format' => '<required>', '$datetime' => '<required>', '$timezone' => '<optional>'] } %sbug01996-002.php:15
%w%f %w%d     -> DateTimeImmutable->__construct($datetime = '2021-07-22') %sbug01996-002.php:18
%w             => $dateTime = class DateTimeImmutable { public $date = '2021-07-22 00:00:00.000000'; public $timezone_type = 3; public $timezone = 'UTC' } %sbug01996-002.php:18
%w             => $closure = class Closure { virtual $closure = "$this->format", public $this = class DateTimeImmutable { public $date = '2021-07-22 00:00:00.000000'; public $timezone_type = 3; public $timezone = 'UTC' }; public $parameter = ['$format' => '<required>'] } %sbug01996-002.php:19
%w%f %w%d     -> xdebug_stop_trace() %sbug01996-002.php:22
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
