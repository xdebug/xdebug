--TEST--
Test with fibonacci numbers
--INI--
xdebug.enable=1
xdebug.auto_trace=0
--FILE--
<?php
	xdebug_start_trace();
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
	xdebug_dump_function_trace();
?>
--EXPECTF--
Function trace:
    %f      %d     -> fibonacci_cache(50) /%s/phpt.%x:22
    %f      %d       -> fibonacci_cache(49) /%s/phpt.%x:16
    %f      %d         -> fibonacci_cache(48) /%s/phpt.%x:16
    %f      %d           -> fibonacci_cache(47) /%s/phpt.%x:16
    %f      %d             -> fibonacci_cache(46) /%s/phpt.%x:16
    %f      %d               -> fibonacci_cache(45) /%s/phpt.%x:16
    %f      %d                 -> fibonacci_cache(44) /%s/phpt.%x:16
    %f      %d                   -> fibonacci_cache(43) /%s/phpt.%x:16
    %f      %d                     -> fibonacci_cache(42) /%s/phpt.%x:16
    %f      %d                       -> fibonacci_cache(41) /%s/phpt.%x:16
    %f      %d                         -> fibonacci_cache(40) /%s/phpt.%x:16
    %f      %d                           -> fibonacci_cache(39) /%s/phpt.%x:16
    %f      %d                             -> fibonacci_cache(38) /%s/phpt.%x:16
    %f      %d                               -> fibonacci_cache(37) /%s/phpt.%x:16
    %f      %d                                 -> fibonacci_cache(36) /%s/phpt.%x:16
    %f      %d                                   -> fibonacci_cache(35) /%s/phpt.%x:16
    %f      %d                                     -> fibonacci_cache(34) /%s/phpt.%x:16
    %f      %d                                       -> fibonacci_cache(33) /%s/phpt.%x:16
    %f      %d                                         -> fibonacci_cache(32) /%s/phpt.%x:16
    %f      %d                                           -> fibonacci_cache(31) /%s/phpt.%x:16
    %f      %d                                             -> fibonacci_cache(30) /%s/phpt.%x:16
    %f      %d                                               -> fibonacci_cache(29) /%s/phpt.%x:16
    %f      %d                                                 -> fibonacci_cache(28) /%s/phpt.%x:16
    %f      %d                                                   -> fibonacci_cache(27) /%s/phpt.%x:16
    %f      %d                                                     -> fibonacci_cache(26) /%s/phpt.%x:16
    %f      %d                                                       -> fibonacci_cache(25) /%s/phpt.%x:16
    %f      %d                                                         -> fibonacci_cache(24) /%s/phpt.%x:16
    %f      %d                                                           -> fibonacci_cache(23) /%s/phpt.%x:16
    %f      %d                                                             -> fibonacci_cache(22) /%s/phpt.%x:16
    %f      %d                                                               -> fibonacci_cache(21) /%s/phpt.%x:16
    %f      %d                                                                 -> fibonacci_cache(20) /%s/phpt.%x:16
    %f      %d                                                                   -> fibonacci_cache(19) /%s/phpt.%x:16
    %f      %d                                                                     -> fibonacci_cache(18) /%s/phpt.%x:16
    %f      %d                                                                       -> fibonacci_cache(17) /%s/phpt.%x:16
    %f      %d                                                                         -> fibonacci_cache(16) /%s/phpt.%x:16
    %f      %d                                                                           -> fibonacci_cache(15) /%s/phpt.%x:16
    %f      %d                                                                             -> fibonacci_cache(14) /%s/phpt.%x:16
    %f      %d                                                                               -> fibonacci_cache(13) /%s/phpt.%x:16
    %f      %d                                                                                 -> fibonacci_cache(12) /%s/phpt.%x:16
    %f      %d                                                                                   -> fibonacci_cache(11) /%s/phpt.%x:16
    %f      %d                                                                                     -> fibonacci_cache(10) /%s/phpt.%x:16
    %f      %d                                                                                       -> fibonacci_cache(9) /%s/phpt.%x:16
    %f      %d                                                                                         -> fibonacci_cache(8) /%s/phpt.%x:16
    %f      %d                                                                                           -> fibonacci_cache(7) /%s/phpt.%x:16
    %f      %d                                                                                             -> fibonacci_cache(6) /%s/phpt.%x:16
    %f      %d                                                                                               -> fibonacci_cache(5) /%s/phpt.%x:16
    %f      %d                                                                                                 -> fibonacci_cache(4) /%s/phpt.%x:16
    %f      %d                                                                                                   -> fibonacci_cache(3) /%s/phpt.%x:16
    %f      %d                                                                                                     -> fibonacci_cache(2) /%s/phpt.%x:16
    %f      %d                                                                                                     -> fibonacci_cache(1) /%s/phpt.%x:16
    %f      %d                                                                                                   -> fibonacci_cache(2) /%s/phpt.%x:16
    %f      %d                                                                                                 -> fibonacci_cache(3) /%s/phpt.%x:16
    %f      %d                                                                                               -> fibonacci_cache(4) /%s/phpt.%x:16
    %f      %d                                                                                             -> fibonacci_cache(5) /%s/phpt.%x:16
    %f      %d                                                                                           -> fibonacci_cache(6) /%s/phpt.%x:16
    %f      %d                                                                                         -> fibonacci_cache(7) /%s/phpt.%x:16
    %f      %d                                                                                       -> fibonacci_cache(8) /%s/phpt.%x:16
    %f      %d                                                                                     -> fibonacci_cache(9) /%s/phpt.%x:16
    %f      %d                                                                                   -> fibonacci_cache(10) /%s/phpt.%x:16
    %f      %d                                                                                 -> fibonacci_cache(11) /%s/phpt.%x:16
    %f      %d                                                                               -> fibonacci_cache(12) /%s/phpt.%x:16
    %f      %d                                                                             -> fibonacci_cache(13) /%s/phpt.%x:16
    %f      %d                                                                           -> fibonacci_cache(14) /%s/phpt.%x:16
    %f      %d                                                                         -> fibonacci_cache(15) /%s/phpt.%x:16
    %f      %d                                                                       -> fibonacci_cache(16) /%s/phpt.%x:16
    %f      %d                                                                     -> fibonacci_cache(17) /%s/phpt.%x:16
    %f      %d                                                                   -> fibonacci_cache(18) /%s/phpt.%x:16
    %f      %d                                                                 -> fibonacci_cache(19) /%s/phpt.%x:16
    %f      %d                                                               -> fibonacci_cache(20) /%s/phpt.%x:16
    %f      %d                                                             -> fibonacci_cache(21) /%s/phpt.%x:16
    %f      %d                                                           -> fibonacci_cache(22) /%s/phpt.%x:16
    %f      %d                                                         -> fibonacci_cache(23) /%s/phpt.%x:16
    %f      %d                                                       -> fibonacci_cache(24) /%s/phpt.%x:16
    %f      %d                                                     -> fibonacci_cache(25) /%s/phpt.%x:16
    %f      %d                                                   -> fibonacci_cache(26) /%s/phpt.%x:16
    %f      %d                                                 -> fibonacci_cache(27) /%s/phpt.%x:16
    %f      %d                                               -> fibonacci_cache(28) /%s/phpt.%x:16
    %f      %d                                             -> fibonacci_cache(29) /%s/phpt.%x:16
    %f      %d                                           -> fibonacci_cache(30) /%s/phpt.%x:16
    %f      %d                                         -> fibonacci_cache(31) /%s/phpt.%x:16
    %f      %d                                       -> fibonacci_cache(32) /%s/phpt.%x:16
    %f      %d                                     -> fibonacci_cache(33) /%s/phpt.%x:16
    %f      %d                                   -> fibonacci_cache(34) /%s/phpt.%x:16
    %f      %d                                 -> fibonacci_cache(35) /%s/phpt.%x:16
    %f      %d                               -> fibonacci_cache(36) /%s/phpt.%x:16
    %f      %d                             -> fibonacci_cache(37) /%s/phpt.%x:16
    %f      %d                           -> fibonacci_cache(38) /%s/phpt.%x:16
    %f      %d                         -> fibonacci_cache(39) /%s/phpt.%x:16
    %f      %d                       -> fibonacci_cache(40) /%s/phpt.%x:16
    %f      %d                     -> fibonacci_cache(41) /%s/phpt.%x:16
    %f      %d                   -> fibonacci_cache(42) /%s/phpt.%x:16
    %f      %d                 -> fibonacci_cache(43) /%s/phpt.%x:16
    %f      %d               -> fibonacci_cache(44) /%s/phpt.%x:16
    %f      %d             -> fibonacci_cache(45) /%s/phpt.%x:16
    %f      %d           -> fibonacci_cache(46) /%s/phpt.%x:16
    %f      %d         -> fibonacci_cache(47) /%s/phpt.%x:16
    %f      %d       -> fibonacci_cache(48) /%s/phpt.%x:16
