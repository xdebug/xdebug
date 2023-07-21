--TEST--
Test for bug #2186: Segfault with trampoline functions and debugger activation
--INI--
xdebug.mode=develop,debug
--FILE--
<?php
$anon = new class() {
	function execute() {
		new ReflectionClass(null);
	}
};

(new Wrapper($anon))->execute(
	(new Wrapper($anon))->execute()
);

class Wrapper
{
	protected $wrapped;

	function __construct($wrapped)
	{
		$this->wrapped = $wrapped;
	}

	public function __call($method, $arguments)
	{
		return call_user_func([$this->wrapped, $method]);
	}
}
?>
--AFTER--
<?php
echo "Done";
?>
--EXPECTF--
%A
Done
