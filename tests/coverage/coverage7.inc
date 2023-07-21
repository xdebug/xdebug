<?php
class foo {
	static function ok($a, $b)
	{
		if ($a && !$b) {
			echo "A NOT B\n";
		} else if (!$a && $b) {
			echo "B NOT A\n";
		}
	}

	function loop_test($a)
	{
		do {
			echo $a, "\n";
		} while( --$a );
	}

	function test_closure()
	{
		$c = function($a, $b) {
			return $a * $b;
		};

		$r = $c(1, 2);
	}
}

foo::ok(true, false);

$f = new foo;
$f->loop_test(2);
$f->test_closure();
?>
