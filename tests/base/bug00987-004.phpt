--TEST--
Test for bug #987: Hidden property names not shown with stack trace (< PHP 7.1)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP < 7.1');
?>
--INI--
html_errors=0
xdebug.cli_color=0
xdebug.mode=display
error_reporting=-1
xdebug.collect_params=4
xdebug.show_local_vars=0
--FILE--
<?php
$object = (object) array('key' => 'value', 1 => 0, -4 => "foo", 3.14 => false);

function foo($a, $b)
{
}

foo($object);
?>
--EXPECTF--
Warning: Missing argument 2 for foo(), called in %sbug00987-004.php on line 8 and defined in %sbug00987-004.php on line 4

Call Stack:
%w%f %w%d   1. {main}() %sbug00987-004.php:0
%w%f %w%d   2. foo($a = class stdClass { public $key = 'value'; public $1 = 0; public $-4 = 'foo'; public $3 = FALSE }, $b = ???) %sbug00987-004.php:8
