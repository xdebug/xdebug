<?php
class Test2055 {
	static protected $c;

	static function setUp()
	{
		self::$c = function($a, $b) { return $a + $b; };
	}

	static function doOp( $a, $b )
	{
		$c = self::$c;

		return $c($a, $b);
	}
}

Test2055::setUp();
echo Test2055::doOp( 5, 8 ), "\n";
