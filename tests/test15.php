<?php
/* $Id: test15.php,v 1.1 2002-06-06 11:05:49 derick Exp $
 * Author: d.rethans@jdimedia.nl
 * Description:
 *   Test for variable member functions
 */
	xdebug_start_trace();

	class a {

		function func_a1() {
		}

		function func_a2() {
		}

	}

	class b {

		function func_b1() {
		}

		function func_b2() {
		}

	}

	$B = new b;
	$B->func_b1();

	$a = 'b';

	xdebug_dump_function_trace();
?>
