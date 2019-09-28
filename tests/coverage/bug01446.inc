<?php
class Test
{
	private $test;

	public function __construct()
	{
		$this->test = (object) [ 'bar' => 42 ];
	}

	public function doSomething()
	{
		if (isset($this->test->foo)) {
			return 'foo';
		} elseif (isset($this->test->bar)) {
			return 'bar';
		}

		return '';
	}
}
?>
