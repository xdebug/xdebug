<?php
	echo xdebug_memory_usage(). "\n";

	for ($i = 0; $i< 10000; $i++) {
		$array[$i] = array ($i);
	}

	echo xdebug_memory_usage(). "\n";

	unset ($array);

	echo xdebug_memory_usage(). "\n";

	for ($i = 0; $i< 10000; $i++) {
		$array[$i] = array ($i);
	}

	echo xdebug_memory_usage(). "\n";

	unset ($array);

	echo xdebug_memory_usage(). "\n";
?>
