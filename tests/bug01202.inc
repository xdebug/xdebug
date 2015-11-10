<?php
$a = new class {
	public $a = 42;
	public function hello() {
		echo 'Hello world';
	}
};

$a->hello();
?>
