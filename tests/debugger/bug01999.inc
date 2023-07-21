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
