<?php
function test($import)
{
	$local = 1;

	$local_two = 'test: ' . $import;

	xdebug_break();
    $local_three = 3;
}

$main_local = 42;
$array = [ 'one', 'two', 'three' ];
array_map( 'test', $array );
