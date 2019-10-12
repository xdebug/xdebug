<?php
class MyClass
{
	function hasFoo()
	{
		return true;
	}
}

function foo(MyClass $class)
{
	return ( $class->hasFoo() &&
		preg_match(
			'((?:^|\s)' . preg_quote( get_class( $class ) ) . '(?:\s|$))',
			get_class( $class )
		)
	);
}

foo(new MyClass);
?>
