<?php
/* $Id: test10.php,v 1.1 2002-05-25 13:44:11 derick Exp $
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
	xdebug_dump_function_trace();
?>
