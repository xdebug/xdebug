--TEST--
Test for bug #1996: Show wrapped callable for closures in traces (< PHP 8.2)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP < 8.2');
?>
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.collect_return=0
xdebug.collect_assignments=1
xdebug.trace_format=0
date.timezone=UTC
--FILE--
<?php
require_once 'capture-trace.inc';


$closure = Closure::fromCallable('substr');
echo $closure("OH NO!", 3);


function user_defined($a, $b)
{
	return substr($a, $b);
}
$closure = Closure::fromCallable('user_defined');
echo $closure("OH NO!", 3);


$closure = Closure::fromCallable(['DateTimeImmutable', 'createFromFormat']);
echo $closure("!Y-m-d", "2021-07-22")->format("Y m d\n");


$dateTime = new DateTimeImmutable("2021-07-22");
$closure = Closure::fromCallable([$dateTime, 'format']);
$closure("Y-m-d\n");


xdebug_stop_trace();
?>
--EXPECTF--
NO!NO!2021 07 22
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w               => $tf = '%sxt%S' %s:%d
%w%f %w%d     -> Closure::fromCallable($call%s = 'substr') %sbug01996-001-php81.php:5
%w             => $closure = class Closure { virtual $closure = "substr", public $parameter = ['$%s' => '<required>', '$%s' => '<required>', '$length' => '<optional>'] } %sbug01996-001-php81.php:5
%w%f %w%d     -> substr($%s = 'OH NO!', $%s = 3) %sbug01996-001-php81.php:6
%w%f %w%d     -> Closure::fromCallable($call%s = 'user_defined') %sbug01996-001-php81.php:13
%w             => $closure = class Closure { virtual $closure = "user_defined", public $parameter = ['$a' => '<required>', '$b' => '<required>'] } %sbug01996-001-php81.php:13
%w%f %w%d     -> user_defined($a = 'OH NO!', $b = 3) %sbug01996-001-php81.php:14
%w%f %w%d       -> substr($%s = 'OH NO!', $%s = 3) %sbug01996-001-php81.php:11
%w%f %w%d     -> Closure::fromCallable($call%s = [0 => 'DateTimeImmutable', 1 => 'createFromFormat']) %sbug01996-001-php81.php:17
%w             => $closure = class Closure { virtual $closure = "DateTimeImmutable::createFromFormat", public $parameter = ['$format' => '<required>', '$%s' => '<required>', '$%s' => '<optional>'] } %sbug01996-001-php81.php:17
%w%f %w%d     -> DateTimeImmutable::createFromFormat($format = '!Y-m-d', $%s = '2021-07-22') %sbug01996-001-php81.php:18
%w%f %w%d     -> DateTimeImmutable->format($format = 'Y m d\n') %sbug01996-001-php81.php:18
%w%f %w%d     -> DateTimeImmutable->__construct($%s = '2021-07-22') %sbug01996-001-php81.php:21
%w             => $dateTime = class DateTimeImmutable { public $date = '2021-07-22 00:00:00.000000'; public $timezone_type = 3; public $timezone = 'UTC' } %sbug01996-001-php81.php:21
%w%f %w%d     -> Closure::fromCallable($call%s = [0 => class DateTimeImmutable { public $date = '2021-07-22 00:00:00.000000'; public $timezone_type = 3; public $timezone = 'UTC' }, 1 => 'format']) %sbug01996-001-php81.php:22
%w             => $closure = class Closure { virtual $closure = "$this->format", public $this = class DateTimeImmutable { public $date = '2021-07-22 00:00:00.000000'; public $timezone_type = 3; public $timezone = 'UTC' }; public $parameter = ['$format' => '<required>'] } %sbug01996-001-php81.php:22
%w%f %w%d     -> DateTimeImmutable->format($format = 'Y-m-d\n') %sbug01996-001-php81.php:23
%w%f %w%d     -> xdebug_stop_trace() %sbug01996-001-php81.php:26
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
