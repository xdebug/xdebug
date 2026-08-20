--TEST--
Test for bug #1999: Show readonly properties (text)
--INI--
xdebug.mode=develop
html_errors=0
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
%sbug01999-text.php:14:
class WithReadOnlyProps#1 (2) {
  public string $static_string =>
  string(3) "two"
  public readonly string $ro_string =>
  string(9) "New Value"
}
