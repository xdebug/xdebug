<?php
/* $Id: test14.php,v 1.1 2002-05-29 14:33:18 derick Exp $
 * Author: d.rethans@jdimedia.nl
 * Description:
 *   Test for circular references
 */
	xdebug_start_trace();

	class foo {

		function foo() {
			$this->a = &$this;
			$this->b = &$this;
		}

	}

	function bar($o) {
	}

	$f = new foo();
	bar($f);
	bar($f);

	xdebug_dump_function_trace();
?>
