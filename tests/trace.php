<?php
	xdebug_start_trace();

	function f2 ($n, $o)
	{
		if ($n > 0) {
			return $x;
		}
	}

	function fibon ($n, $o)
	{
		$x = f2 ($d, $o);
		return $x;
	}

	fibon (8, 5);

	xdebug_get_function_trace();
	xdebug_stop_trace();
?>
