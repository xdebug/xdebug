<?php
class pow {}

class Test
{
	static $foo = 42;

	public function doSomething()
	{
		new pow(
			static::$foo,
			3
		);
	}
}
?>
