--TEST--
Test for bug #987: Hidden property names not shown with stack trace. (< PHP 7.1)
--SKIPIF--
<?php if (!version_compare(phpversion(), "7.1", '<')) echo "skip < PHP 7.1 needed\n"; ?>
--INI--
html_errors=0
xdebug.cli_color=0
xdebug.default_enable=1
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
    %f     %d   1. {main}() %sbug00987-004.php:0
    %f     %d   2. foo($a = class stdClass { public $key = 'value'; public $1 = 0; public $-4 = 'foo'; public $3 = FALSE }, $b = ???) %sbug00987-004.php:8
