<?php
$var = [];
for ( $i = 0; $i < 32; $i++ )
{
	$var[ "{$i}-" . chr( $i ) . "-{$i}" ] = "v";
}
var_dump($var);
