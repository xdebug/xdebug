<?php
/* $Id: test13.php,v 1.1 2002-05-26 11:00:16 derick Exp $
 * Author: d.rethans@jdimedia.nl
 * Description:
 *   Test for variable function calls
 */
	xdebug_start_trace();

	function foo1 ($a)
	{
		return addslashes ($a);
	}

	function foo2 ($a)
	{
		return addslashes ($a);
	}

	function foo3 ($a)
	{
		return addslashes ($a);
	}

	function foo4 ($a)
	{
		return addslashes ($a);
	}

	$f = 'foo1';
	$f('test\'s');
	$g = 'foo4';
	$g('test\'s');
	$h = 'foo2';
	$h('test\'s');
	$i = 'foo3';
	$i('test\'s');

	xdebug_dump_function_trace();
?>
	
