<?php

function loadMod( $module )
{
	strrev( $module );
	$module .= 's';
	strrev( $module );
}

function loadFoo( $test )
{
	strrev( $test );
	$test .= 's';
	strrev( $test );
}

loadFoo( 'view' );
loadMod( 'test' );
loadMod( 'view' );
