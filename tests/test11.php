<?php
/* $Id: test11.php,v 1.1 2002-05-25 13:42:52 derick Exp $
 * Author: d.rethans@jdimedia.nl
 * Description:
 *   Test for indirect function calls
 */
	xdebug_start_trace();


	function blaat ()
	{
	}

	$func = 'blaat';
	echo $func();

	xdebug_dump_function_trace();
?>
