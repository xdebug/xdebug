--TEST--
xdebug_time_index()
--INI--
xdebug.mode=develop
--FILE--
<?php
usleep( 250000 );

$rq = $_SERVER['REQUEST_TIME_FLOAT'];
$c  = microtime( true );
$xt = xdebug_time_index();

$d  = ($rq + $xt) - $c;

echo "Request time (float): ", $rq, "\n";
echo "Xdebug time index:    ", $xt, "\n";
echo "Current microtime:    ", $c,  "\n";
echo "Difference:           ", $d,  "\n";

echo "The difference is ", abs($d) > 1e-2 ? "too high\n" : "fine\n";
?>
--EXPECTF--
Request time (float): 1%d.%d
Xdebug time index:    0.%r(2|3|4)%r%d
Current microtime:    1%d.%d
Difference:           %f
The difference is fine

