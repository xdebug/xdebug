<?php
/* $Id: test12.php,v 1.1 2002-05-25 13:42:52 derick Exp $
 * Author: d.rethans@jdimedia.nl
 * Description:
 *   Test for complex parameters to functions
 */
	xdebug_start_trace();

	function a ($a, $b, $h, &$i) {
		echo $a;
		return $a + $b;
	}

	$a = array (1, 2,3,4,5);
	$b = array ("h" => 9.12, $a, $a, $a, "p" => 9 - 0.12);
	echo a (5, 9.12, FALSE, $b);

	xdebug_dump_function_trace();
?>
