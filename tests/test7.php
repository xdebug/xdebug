<?php
	xdebug_start_trace();

	class aaa {
		function a1 () {
			return 'a1';
		}
		function a2 () {
			return 'a2';
		}
	}

	class bbb {
		function b1 () {
			return 'a1';
		}
		function b2 () {
			return 'a2';
		}
	}


	$a = new aaa;
	$a->a2();

	$b = new bbb;
	$b->b2();
	$a = new aaa;
	$a->a2();

	xdebug_dump_function_trace();
?>

