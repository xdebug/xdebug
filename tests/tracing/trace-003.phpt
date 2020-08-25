--TEST--
Trace test with fibonacci numbers (format=0)
--INI--
xdebug.mode=trace
xdebug.start_with_request=0
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.trace_format=0
--FILE--
<?php
	$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));
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
	xdebug_stop_trace();
	echo file_get_contents($tf);
	unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> fibonacci_cache($n = 50) %strace-003.php:22
%w%f %w%d       -> fibonacci_cache($n = 49) %strace-003.php:16
%w%f %w%d         -> fibonacci_cache($n = 48) %strace-003.php:16
%w%f %w%d           -> fibonacci_cache($n = 47) %strace-003.php:16
%w%f %w%d             -> fibonacci_cache($n = 46) %strace-003.php:16
%w%f %w%d               -> fibonacci_cache($n = 45) %strace-003.php:16
%w%f %w%d                 -> fibonacci_cache($n = 44) %strace-003.php:16
%w%f %w%d                   -> fibonacci_cache($n = 43) %strace-003.php:16
%w%f %w%d                     -> fibonacci_cache($n = 42) %strace-003.php:16
%w%f %w%d                       -> fibonacci_cache($n = 41) %strace-003.php:16
%w%f %w%d                         -> fibonacci_cache($n = 40) %strace-003.php:16
%w%f %w%d                           -> fibonacci_cache($n = 39) %strace-003.php:16
%w%f %w%d                             -> fibonacci_cache($n = 38) %strace-003.php:16
%w%f %w%d                               -> fibonacci_cache($n = 37) %strace-003.php:16
%w%f %w%d                                 -> fibonacci_cache($n = 36) %strace-003.php:16
%w%f %w%d                                   -> fibonacci_cache($n = 35) %strace-003.php:16
%w%f %w%d                                     -> fibonacci_cache($n = 34) %strace-003.php:16
%w%f %w%d                                       -> fibonacci_cache($n = 33) %strace-003.php:16
%w%f %w%d                                         -> fibonacci_cache($n = 32) %strace-003.php:16
%w%f %w%d                                           -> fibonacci_cache($n = 31) %strace-003.php:16
%w%f %w%d                                             -> fibonacci_cache($n = 30) %strace-003.php:16
%w%f %w%d                                               -> fibonacci_cache($n = 29) %strace-003.php:16
%w%f %w%d                                                 -> fibonacci_cache($n = 28) %strace-003.php:16
%w%f %w%d                                                   -> fibonacci_cache($n = 27) %strace-003.php:16
%w%f %w%d                                                     -> fibonacci_cache($n = 26) %strace-003.php:16
%w%f %w%d                                                       -> fibonacci_cache($n = 25) %strace-003.php:16
%w%f %w%d                                                         -> fibonacci_cache($n = 24) %strace-003.php:16
%w%f %w%d                                                           -> fibonacci_cache($n = 23) %strace-003.php:16
%w%f %w%d                                                             -> fibonacci_cache($n = 22) %strace-003.php:16
%w%f %w%d                                                               -> fibonacci_cache($n = 21) %strace-003.php:16
%w%f %w%d                                                                 -> fibonacci_cache($n = 20) %strace-003.php:16
%w%f %w%d                                                                   -> fibonacci_cache($n = 19) %strace-003.php:16
%w%f %w%d                                                                     -> fibonacci_cache($n = 18) %strace-003.php:16
%w%f %w%d                                                                       -> fibonacci_cache($n = 17) %strace-003.php:16
%w%f %w%d                                                                         -> fibonacci_cache($n = 16) %strace-003.php:16
%w%f %w%d                                                                           -> fibonacci_cache($n = 15) %strace-003.php:16
%w%f %w%d                                                                             -> fibonacci_cache($n = 14) %strace-003.php:16
%w%f %w%d                                                                               -> fibonacci_cache($n = 13) %strace-003.php:16
%w%f %w%d                                                                                 -> fibonacci_cache($n = 12) %strace-003.php:16
%w%f %w%d                                                                                   -> fibonacci_cache($n = 11) %strace-003.php:16
%w%f %w%d                                                                                     -> fibonacci_cache($n = 10) %strace-003.php:16
%w%f %w%d                                                                                       -> fibonacci_cache($n = 9) %strace-003.php:16
%w%f %w%d                                                                                         -> fibonacci_cache($n = 8) %strace-003.php:16
%w%f %w%d                                                                                           -> fibonacci_cache($n = 7) %strace-003.php:16
%w%f %w%d                                                                                             -> fibonacci_cache($n = 6) %strace-003.php:16
%w%f %w%d                                                                                               -> fibonacci_cache($n = 5) %strace-003.php:16
%w%f %w%d                                                                                                 -> fibonacci_cache($n = 4) %strace-003.php:16
%w%f %w%d                                                                                                   -> fibonacci_cache($n = 3) %strace-003.php:16
%w%f %w%d                                                                                                     -> fibonacci_cache($n = 2) %strace-003.php:16
%w%f %w%d                                                                                                     -> fibonacci_cache($n = 1) %strace-003.php:16
%w%f %w%d                                                                                                   -> fibonacci_cache($n = 2) %strace-003.php:16
%w%f %w%d                                                                                                 -> fibonacci_cache($n = 3) %strace-003.php:16
%w%f %w%d                                                                                               -> fibonacci_cache($n = 4) %strace-003.php:16
%w%f %w%d                                                                                             -> fibonacci_cache($n = 5) %strace-003.php:16
%w%f %w%d                                                                                           -> fibonacci_cache($n = 6) %strace-003.php:16
%w%f %w%d                                                                                         -> fibonacci_cache($n = 7) %strace-003.php:16
%w%f %w%d                                                                                       -> fibonacci_cache($n = 8) %strace-003.php:16
%w%f %w%d                                                                                     -> fibonacci_cache($n = 9) %strace-003.php:16
%w%f %w%d                                                                                   -> fibonacci_cache($n = 10) %strace-003.php:16
%w%f %w%d                                                                                 -> fibonacci_cache($n = 11) %strace-003.php:16
%w%f %w%d                                                                               -> fibonacci_cache($n = 12) %strace-003.php:16
%w%f %w%d                                                                             -> fibonacci_cache($n = 13) %strace-003.php:16
%w%f %w%d                                                                           -> fibonacci_cache($n = 14) %strace-003.php:16
%w%f %w%d                                                                         -> fibonacci_cache($n = 15) %strace-003.php:16
%w%f %w%d                                                                       -> fibonacci_cache($n = 16) %strace-003.php:16
%w%f %w%d                                                                     -> fibonacci_cache($n = 17) %strace-003.php:16
%w%f %w%d                                                                   -> fibonacci_cache($n = 18) %strace-003.php:16
%w%f %w%d                                                                 -> fibonacci_cache($n = 19) %strace-003.php:16
%w%f %w%d                                                               -> fibonacci_cache($n = 20) %strace-003.php:16
%w%f %w%d                                                             -> fibonacci_cache($n = 21) %strace-003.php:16
%w%f %w%d                                                           -> fibonacci_cache($n = 22) %strace-003.php:16
%w%f %w%d                                                         -> fibonacci_cache($n = 23) %strace-003.php:16
%w%f %w%d                                                       -> fibonacci_cache($n = 24) %strace-003.php:16
%w%f %w%d                                                     -> fibonacci_cache($n = 25) %strace-003.php:16
%w%f %w%d                                                   -> fibonacci_cache($n = 26) %strace-003.php:16
%w%f %w%d                                                 -> fibonacci_cache($n = 27) %strace-003.php:16
%w%f %w%d                                               -> fibonacci_cache($n = 28) %strace-003.php:16
%w%f %w%d                                             -> fibonacci_cache($n = 29) %strace-003.php:16
%w%f %w%d                                           -> fibonacci_cache($n = 30) %strace-003.php:16
%w%f %w%d                                         -> fibonacci_cache($n = 31) %strace-003.php:16
%w%f %w%d                                       -> fibonacci_cache($n = 32) %strace-003.php:16
%w%f %w%d                                     -> fibonacci_cache($n = 33) %strace-003.php:16
%w%f %w%d                                   -> fibonacci_cache($n = 34) %strace-003.php:16
%w%f %w%d                                 -> fibonacci_cache($n = 35) %strace-003.php:16
%w%f %w%d                               -> fibonacci_cache($n = 36) %strace-003.php:16
%w%f %w%d                             -> fibonacci_cache($n = 37) %strace-003.php:16
%w%f %w%d                           -> fibonacci_cache($n = 38) %strace-003.php:16
%w%f %w%d                         -> fibonacci_cache($n = 39) %strace-003.php:16
%w%f %w%d                       -> fibonacci_cache($n = 40) %strace-003.php:16
%w%f %w%d                     -> fibonacci_cache($n = 41) %strace-003.php:16
%w%f %w%d                   -> fibonacci_cache($n = 42) %strace-003.php:16
%w%f %w%d                 -> fibonacci_cache($n = 43) %strace-003.php:16
%w%f %w%d               -> fibonacci_cache($n = 44) %strace-003.php:16
%w%f %w%d             -> fibonacci_cache($n = 45) %strace-003.php:16
%w%f %w%d           -> fibonacci_cache($n = 46) %strace-003.php:16
%w%f %w%d         -> fibonacci_cache($n = 47) %strace-003.php:16
%w%f %w%d       -> fibonacci_cache($n = 48) %strace-003.php:16
%w%f %w%d     -> xdebug_stop_trace() %strace-003.php:23
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
