<?php
/* $Id: test10.php,v 1.2 2002-06-09 07:45:51 derick Exp $
 * Author: d.rethans@jdimedia.nl
 * Description:
 *   Test for functions in parameters to functions
 */

	xdebug_start_trace();
	class D
	{
		function a($x) {
			return 'a';
		}
		function b($x) {
			return 'b';
		}
		function c($x) {
			return 'c';
		}
	}

	function blaat($a) {
	}

	blaat("insert blah '".D::a(D::b(D::a(D::c('blah')))));
	var_dump (xdebug_get_function_trace());
?>
