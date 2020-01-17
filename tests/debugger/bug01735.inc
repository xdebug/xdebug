<?php
error_reporting( -1 );
function error_handler( int $errno, string $errstr, ...$rest)
{
	echo "IN ERROR HANDLER\n";
	throw new ErrorException( $errstr );
}

set_error_handler( 'error_handler' );

$variable1 = 42;
$variable2 = 42;
$variable3 = 42;
?>
