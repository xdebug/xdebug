--TEST--
Test with fibonacci numbers
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
xdebug.collect_return=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.show_mem_delta=0
xdebug.trace_format=0
--FILE--
<?php
	$tf = xdebug_start_trace('/tmp/'. uniqid('xdt', TRUE));
    function fibonacci_cache ($n)
    {
        if (isset ($GLOBALS['fcache'][$n])) {
            return $GLOBALS['fcache'][$n];
        }

        if ($n == 0) {
            return 0;
        } else if ($n == 1) {
            return 1;
        } else if ($n == 2) {
            return 1;
        } else {
            $t = fibonacci_cache($n - 1) + fibonacci_cache($n - 2);
            $GLOBALS['fcache'][$n] = $t;
            return $t;
        }
    }

	fibonacci_cache(50);
	echo file_get_contents($tf);
	unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d]
    %f      %d     -> fibonacci_cache(50) /%s/trace.php:22
    %f      %d       -> fibonacci_cache(49) /%s/trace.php:16
    %f      %d         -> fibonacci_cache(48) /%s/trace.php:16
    %f      %d           -> fibonacci_cache(47) /%s/trace.php:16
    %f      %d             -> fibonacci_cache(46) /%s/trace.php:16
    %f      %d               -> fibonacci_cache(45) /%s/trace.php:16
    %f      %d                 -> fibonacci_cache(44) /%s/trace.php:16
    %f      %d                   -> fibonacci_cache(43) /%s/trace.php:16
    %f      %d                     -> fibonacci_cache(42) /%s/trace.php:16
    %f      %d                       -> fibonacci_cache(41) /%s/trace.php:16
    %f      %d                         -> fibonacci_cache(40) /%s/trace.php:16
    %f      %d                           -> fibonacci_cache(39) /%s/trace.php:16
    %f      %d                             -> fibonacci_cache(38) /%s/trace.php:16
    %f      %d                               -> fibonacci_cache(37) /%s/trace.php:16
    %f      %d                                 -> fibonacci_cache(36) /%s/trace.php:16
    %f      %d                                   -> fibonacci_cache(35) /%s/trace.php:16
    %f      %d                                     -> fibonacci_cache(34) /%s/trace.php:16
    %f      %d                                       -> fibonacci_cache(33) /%s/trace.php:16
    %f      %d                                         -> fibonacci_cache(32) /%s/trace.php:16
    %f      %d                                           -> fibonacci_cache(31) /%s/trace.php:16
    %f      %d                                             -> fibonacci_cache(30) /%s/trace.php:16
    %f      %d                                               -> fibonacci_cache(29) /%s/trace.php:16
    %f      %d                                                 -> fibonacci_cache(28) /%s/trace.php:16
    %f      %d                                                   -> fibonacci_cache(27) /%s/trace.php:16
    %f      %d                                                     -> fibonacci_cache(26) /%s/trace.php:16
    %f      %d                                                       -> fibonacci_cache(25) /%s/trace.php:16
    %f      %d                                                         -> fibonacci_cache(24) /%s/trace.php:16
    %f      %d                                                           -> fibonacci_cache(23) /%s/trace.php:16
    %f      %d                                                             -> fibonacci_cache(22) /%s/trace.php:16
    %f      %d                                                               -> fibonacci_cache(21) /%s/trace.php:16
    %f      %d                                                                 -> fibonacci_cache(20) /%s/trace.php:16
    %f      %d                                                                   -> fibonacci_cache(19) /%s/trace.php:16
    %f      %d                                                                     -> fibonacci_cache(18) /%s/trace.php:16
    %f      %d                                                                       -> fibonacci_cache(17) /%s/trace.php:16
    %f      %d                                                                         -> fibonacci_cache(16) /%s/trace.php:16
    %f      %d                                                                           -> fibonacci_cache(15) /%s/trace.php:16
    %f      %d                                                                             -> fibonacci_cache(14) /%s/trace.php:16
    %f      %d                                                                               -> fibonacci_cache(13) /%s/trace.php:16
    %f      %d                                                                                 -> fibonacci_cache(12) /%s/trace.php:16
    %f      %d                                                                                   -> fibonacci_cache(11) /%s/trace.php:16
    %f      %d                                                                                     -> fibonacci_cache(10) /%s/trace.php:16
    %f      %d                                                                                       -> fibonacci_cache(9) /%s/trace.php:16
    %f      %d                                                                                         -> fibonacci_cache(8) /%s/trace.php:16
    %f      %d                                                                                           -> fibonacci_cache(7) /%s/trace.php:16
    %f      %d                                                                                             -> fibonacci_cache(6) /%s/trace.php:16
    %f      %d                                                                                               -> fibonacci_cache(5) /%s/trace.php:16
    %f      %d                                                                                                 -> fibonacci_cache(4) /%s/trace.php:16
    %f      %d                                                                                                   -> fibonacci_cache(3) /%s/trace.php:16
    %f      %d                                                                                                     -> fibonacci_cache(2) /%s/trace.php:16
    %f      %d                                                                                                     -> fibonacci_cache(1) /%s/trace.php:16
    %f      %d                                                                                                   -> fibonacci_cache(2) /%s/trace.php:16
    %f      %d                                                                                                 -> fibonacci_cache(3) /%s/trace.php:16
    %f      %d                                                                                               -> fibonacci_cache(4) /%s/trace.php:16
    %f      %d                                                                                             -> fibonacci_cache(5) /%s/trace.php:16
    %f      %d                                                                                           -> fibonacci_cache(6) /%s/trace.php:16
    %f      %d                                                                                         -> fibonacci_cache(7) /%s/trace.php:16
    %f      %d                                                                                       -> fibonacci_cache(8) /%s/trace.php:16
    %f      %d                                                                                     -> fibonacci_cache(9) /%s/trace.php:16
    %f      %d                                                                                   -> fibonacci_cache(10) /%s/trace.php:16
    %f      %d                                                                                 -> fibonacci_cache(11) /%s/trace.php:16
    %f      %d                                                                               -> fibonacci_cache(12) /%s/trace.php:16
    %f      %d                                                                             -> fibonacci_cache(13) /%s/trace.php:16
    %f      %d                                                                           -> fibonacci_cache(14) /%s/trace.php:16
    %f      %d                                                                         -> fibonacci_cache(15) /%s/trace.php:16
    %f      %d                                                                       -> fibonacci_cache(16) /%s/trace.php:16
    %f      %d                                                                     -> fibonacci_cache(17) /%s/trace.php:16
    %f      %d                                                                   -> fibonacci_cache(18) /%s/trace.php:16
    %f      %d                                                                 -> fibonacci_cache(19) /%s/trace.php:16
    %f      %d                                                               -> fibonacci_cache(20) /%s/trace.php:16
    %f      %d                                                             -> fibonacci_cache(21) /%s/trace.php:16
    %f      %d                                                           -> fibonacci_cache(22) /%s/trace.php:16
    %f      %d                                                         -> fibonacci_cache(23) /%s/trace.php:16
    %f      %d                                                       -> fibonacci_cache(24) /%s/trace.php:16
    %f      %d                                                     -> fibonacci_cache(25) /%s/trace.php:16
    %f      %d                                                   -> fibonacci_cache(26) /%s/trace.php:16
    %f      %d                                                 -> fibonacci_cache(27) /%s/trace.php:16
    %f      %d                                               -> fibonacci_cache(28) /%s/trace.php:16
    %f      %d                                             -> fibonacci_cache(29) /%s/trace.php:16
    %f      %d                                           -> fibonacci_cache(30) /%s/trace.php:16
    %f      %d                                         -> fibonacci_cache(31) /%s/trace.php:16
    %f      %d                                       -> fibonacci_cache(32) /%s/trace.php:16
    %f      %d                                     -> fibonacci_cache(33) /%s/trace.php:16
    %f      %d                                   -> fibonacci_cache(34) /%s/trace.php:16
    %f      %d                                 -> fibonacci_cache(35) /%s/trace.php:16
    %f      %d                               -> fibonacci_cache(36) /%s/trace.php:16
    %f      %d                             -> fibonacci_cache(37) /%s/trace.php:16
    %f      %d                           -> fibonacci_cache(38) /%s/trace.php:16
    %f      %d                         -> fibonacci_cache(39) /%s/trace.php:16
    %f      %d                       -> fibonacci_cache(40) /%s/trace.php:16
    %f      %d                     -> fibonacci_cache(41) /%s/trace.php:16
    %f      %d                   -> fibonacci_cache(42) /%s/trace.php:16
    %f      %d                 -> fibonacci_cache(43) /%s/trace.php:16
    %f      %d               -> fibonacci_cache(44) /%s/trace.php:16
    %f      %d             -> fibonacci_cache(45) /%s/trace.php:16
    %f      %d           -> fibonacci_cache(46) /%s/trace.php:16
    %f      %d         -> fibonacci_cache(47) /%s/trace.php:16
    %f      %d       -> fibonacci_cache(48) /%s/trace.php:16
    %f      %d     -> file_get_contents('/tmp/%s') /%s/trace.php:23
