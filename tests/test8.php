<?php
xdebug_start_trace();

function a () {
	var_dump (xdebug_get_function_stack());
	return 'a';
}

function b ($b) {
	return $b.'b';
}

function c ($a, $b) {
	return $b.$a;
}

echo c(c(a(),b(2)), c(a(), a()));

xdebug_dump_function_trace();

?>
