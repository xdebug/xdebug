<?php
class foo {
	function breakpoint1()
	{
		$a = array( // line #1
			1 => "first", // line #2
			2 => "second",
		);
	}

	static function breakpoint2()
	{
		$a = array( // line #1
			1 => "first", // line #2
			2 => "second",
		);
	}
}

$f = new foo;
$f->breakpoint1();

foo::breakpoint2();
?>
