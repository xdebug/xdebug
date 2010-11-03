<?php

function loadMod( $module )
{
	strlen( $module );
	$module .= 's';
	strlen( $module );
}

function loadFoo( $test )
{
	strlen( $test );
	$test .= 's';
	strlen( $test );
}

loadFoo( 'view' );
loadMod( 'test' );
loadMod( 'view' );
