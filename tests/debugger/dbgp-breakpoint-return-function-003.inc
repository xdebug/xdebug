<?php
class Foo
{
	public $x;
	public $y;

	public function __construct($x, $y)
	{
		$this->x = $x;
		$this->y = $y;
	}

	public function product()
	{
		$ocean = 'yes';
		return $this->x * $this->y;
	}
}

function breakOnMe()
{
	return new Foo(42, 2.7);
}

$foo = breakOnMe();

echo $foo->product(), "\n";
?>
