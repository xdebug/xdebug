--TEST--
Test for bug #1999: Show readonly properties (PHP >= 8.1, ansi)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.1');
?>
--INI--
xdebug.mode=develop
html_errors=0
xdebug.cli_color=2
--FILE--
<?php
class WithReadOnlyProps
{
	static int $static_int = 1;

	function __construct(
		public string $static_string = "two",
		public readonly string $ro_string = "readonly-default",
	) {}
}

$obj = new WithReadOnlyProps(ro_string: "New Value");

var_dump($obj);
?>
--EXPECTF--
[1m%sbug01999-ansi.php[22m:[1m14[22m:
[1mclass[22m [31mWithReadOnlyProps[0m#1 ([32m2[0m) {
  [32m[1mpublic[22m string[0m $static_string [0m=>[0m
  [1mstring[22m([32m3[0m) "[31mtwo[0m"
  [32m[1mpublic[22m readonly string[0m $ro_string [0m=>[0m
  [1mstring[22m([32m9[0m) "[31mNew Value[0m"
}
