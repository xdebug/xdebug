<?php
/* $Id: test16.php,v 1.1 2002-06-06 11:05:49 derick Exp $
 * Author: d.rethans@jdimedia.nl
 * Description:
 *   Test for overloaded member functions / classes
 */
	xdebug_start_trace();

	class a {

		function func_a1() {
		}

		function func_a2() {
		}

	}

	class b extends a {

		function func_b1() {
		}

		function func_b2() {
		}

	}

	$B = new b;
	$B->func_a1();
	$B->func_b1();

	xdebug_dump_function_trace();
?>
