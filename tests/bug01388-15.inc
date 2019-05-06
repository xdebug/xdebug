<?php
function test1()
{
	var_dump( "begin test1" );
	var_dump( "end test1" );

	return "return from test1";
}

class test_class
{
	static function test2()
	{
		var_dump( "begin test2" );
		var_dump( "end test2" );

		return "return from test2";
	}

	function test3()
	{
		var_dump( "begin test3" );
		var_dump( "end test3" );

		return "return from test3";
	}
}

test1();
test_class::test2();

$test = new test_class;
$test->test3();
