<?php
/* $Id: test17.php,v 1.2 2002-08-30 06:18:47 derick Exp $
 * Author: d.rethans@jdimedia.nl
 * Description:
 *   Test for internal parameters
 */

	xdebug_start_trace();

	echo str_repeat ("5", 5);

	xdebug_dump_function_trace();
?>
