--TEST--
xdebug_time_index() with TSC clock (for profiler on Windows and x86 platform only)
--INI--
xdebug.mode=develop,profile
xdebug.start_with_request=default
xdebug.profiler_output_name=cachegrind.out.%p.%r
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
Xdebug time index:    0.%r(2|3)%r%d
Current microtime:    1%d.%d
Difference:           %f
The difference is fine

