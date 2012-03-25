<?php
class Base {
	static public $property = true;
	
	static public function basetest( ) {
		echo 'blah blah blah';
	}
}

class Test extends Base {
	public function test( ) {
		static::basetest( );
	}
}

$test = new Test;
$test->test( );
