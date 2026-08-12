<?php
$before = microtime(true);
xdebug_connect_to_client();
$after = microtime(true);

$start20260812 = 1786492800;

printf(
	"Before: %f\nAfter:  %f\nDiff:   %f\n\n",
	$before - $start20260812,
	$after - $start20260812,
	$after - $before);
?>
