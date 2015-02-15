<?php
class foo {
	function scope0() {
		$var = 'scope0';
		$break = null;
	}

	function scope1() {
		$var = 'scope1';
		$this->scope0();
	}

	function scope2() {
		$var = 'scope2';
		$this->scope1();
	}
}

$f = new foo;
$f->scope2();

?>
