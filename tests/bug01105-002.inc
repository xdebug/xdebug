<?php
class foo {
	static function scope0() {
		$var = 'scope0';
		$break = null;
	}

	static function scope1() {
		$var = 'scope1';
		self::scope0();
	}

	static function scope2() {
		$var = 'scope2';
		self::scope1();
	}
}

foo::scope2();

?>
