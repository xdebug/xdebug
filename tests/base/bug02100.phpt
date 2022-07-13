--TEST--
Test for bug #2100: Exceptions in __debugInfo() can cause issues
--FILE--
<?php
class test
{
	public function throw(test $object)
	{
		throw new Exception('X');
	}

	public function __debugInfo()
	{
		throw new Exception('Y');
		return [];
	}
}

try {
	$t1 = new test;
	$t1->throw(new test);
} catch (Exception $e) {
	echo 'All is well';
}
?>
--EXPECT--
All is well
