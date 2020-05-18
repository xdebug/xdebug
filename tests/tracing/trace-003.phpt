--TEST--
Trace test with fibonacci numbers (format=0)
--INI--
xdebug.mode=trace
xdebug.start_with_request=0
xdebug.collect_params=3
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.show_mem_delta=0
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
TRACE START [%d-%d-%d %d:%d:%d]
%w%f %w%d     -> fibonacci_cache(50) %strace-003.php:22
%w%f %w%d       -> fibonacci_cache(49) %strace-003.php:16
%w%f %w%d         -> fibonacci_cache(48) %strace-003.php:16
%w%f %w%d           -> fibonacci_cache(47) %strace-003.php:16
%w%f %w%d             -> fibonacci_cache(46) %strace-003.php:16
%w%f %w%d               -> fibonacci_cache(45) %strace-003.php:16
%w%f %w%d                 -> fibonacci_cache(44) %strace-003.php:16
%w%f %w%d                   -> fibonacci_cache(43) %strace-003.php:16
%w%f %w%d                     -> fibonacci_cache(42) %strace-003.php:16
%w%f %w%d                       -> fibonacci_cache(41) %strace-003.php:16
%w%f %w%d                         -> fibonacci_cache(40) %strace-003.php:16
%w%f %w%d                           -> fibonacci_cache(39) %strace-003.php:16
%w%f %w%d                             -> fibonacci_cache(38) %strace-003.php:16
%w%f %w%d                               -> fibonacci_cache(37) %strace-003.php:16
%w%f %w%d                                 -> fibonacci_cache(36) %strace-003.php:16
%w%f %w%d                                   -> fibonacci_cache(35) %strace-003.php:16
%w%f %w%d                                     -> fibonacci_cache(34) %strace-003.php:16
%w%f %w%d                                       -> fibonacci_cache(33) %strace-003.php:16
%w%f %w%d                                         -> fibonacci_cache(32) %strace-003.php:16
%w%f %w%d                                           -> fibonacci_cache(31) %strace-003.php:16
%w%f %w%d                                             -> fibonacci_cache(30) %strace-003.php:16
%w%f %w%d                                               -> fibonacci_cache(29) %strace-003.php:16
%w%f %w%d                                                 -> fibonacci_cache(28) %strace-003.php:16
%w%f %w%d                                                   -> fibonacci_cache(27) %strace-003.php:16
%w%f %w%d                                                     -> fibonacci_cache(26) %strace-003.php:16
%w%f %w%d                                                       -> fibonacci_cache(25) %strace-003.php:16
%w%f %w%d                                                         -> fibonacci_cache(24) %strace-003.php:16
%w%f %w%d                                                           -> fibonacci_cache(23) %strace-003.php:16
%w%f %w%d                                                             -> fibonacci_cache(22) %strace-003.php:16
%w%f %w%d                                                               -> fibonacci_cache(21) %strace-003.php:16
%w%f %w%d                                                                 -> fibonacci_cache(20) %strace-003.php:16
%w%f %w%d                                                                   -> fibonacci_cache(19) %strace-003.php:16
%w%f %w%d                                                                     -> fibonacci_cache(18) %strace-003.php:16
%w%f %w%d                                                                       -> fibonacci_cache(17) %strace-003.php:16
%w%f %w%d                                                                         -> fibonacci_cache(16) %strace-003.php:16
%w%f %w%d                                                                           -> fibonacci_cache(15) %strace-003.php:16
%w%f %w%d                                                                             -> fibonacci_cache(14) %strace-003.php:16
%w%f %w%d                                                                               -> fibonacci_cache(13) %strace-003.php:16
%w%f %w%d                                                                                 -> fibonacci_cache(12) %strace-003.php:16
%w%f %w%d                                                                                   -> fibonacci_cache(11) %strace-003.php:16
%w%f %w%d                                                                                     -> fibonacci_cache(10) %strace-003.php:16
%w%f %w%d                                                                                       -> fibonacci_cache(9) %strace-003.php:16
%w%f %w%d                                                                                         -> fibonacci_cache(8) %strace-003.php:16
%w%f %w%d                                                                                           -> fibonacci_cache(7) %strace-003.php:16
%w%f %w%d                                                                                             -> fibonacci_cache(6) %strace-003.php:16
%w%f %w%d                                                                                               -> fibonacci_cache(5) %strace-003.php:16
%w%f %w%d                                                                                                 -> fibonacci_cache(4) %strace-003.php:16
%w%f %w%d                                                                                                   -> fibonacci_cache(3) %strace-003.php:16
%w%f %w%d                                                                                                     -> fibonacci_cache(2) %strace-003.php:16
%w%f %w%d                                                                                                     -> fibonacci_cache(1) %strace-003.php:16
%w%f %w%d                                                                                                   -> fibonacci_cache(2) %strace-003.php:16
%w%f %w%d                                                                                                 -> fibonacci_cache(3) %strace-003.php:16
%w%f %w%d                                                                                               -> fibonacci_cache(4) %strace-003.php:16
%w%f %w%d                                                                                             -> fibonacci_cache(5) %strace-003.php:16
%w%f %w%d                                                                                           -> fibonacci_cache(6) %strace-003.php:16
%w%f %w%d                                                                                         -> fibonacci_cache(7) %strace-003.php:16
%w%f %w%d                                                                                       -> fibonacci_cache(8) %strace-003.php:16
%w%f %w%d                                                                                     -> fibonacci_cache(9) %strace-003.php:16
%w%f %w%d                                                                                   -> fibonacci_cache(10) %strace-003.php:16
%w%f %w%d                                                                                 -> fibonacci_cache(11) %strace-003.php:16
%w%f %w%d                                                                               -> fibonacci_cache(12) %strace-003.php:16
%w%f %w%d                                                                             -> fibonacci_cache(13) %strace-003.php:16
%w%f %w%d                                                                           -> fibonacci_cache(14) %strace-003.php:16
%w%f %w%d                                                                         -> fibonacci_cache(15) %strace-003.php:16
%w%f %w%d                                                                       -> fibonacci_cache(16) %strace-003.php:16
%w%f %w%d                                                                     -> fibonacci_cache(17) %strace-003.php:16
%w%f %w%d                                                                   -> fibonacci_cache(18) %strace-003.php:16
%w%f %w%d                                                                 -> fibonacci_cache(19) %strace-003.php:16
%w%f %w%d                                                               -> fibonacci_cache(20) %strace-003.php:16
%w%f %w%d                                                             -> fibonacci_cache(21) %strace-003.php:16
%w%f %w%d                                                           -> fibonacci_cache(22) %strace-003.php:16
%w%f %w%d                                                         -> fibonacci_cache(23) %strace-003.php:16
%w%f %w%d                                                       -> fibonacci_cache(24) %strace-003.php:16
%w%f %w%d                                                     -> fibonacci_cache(25) %strace-003.php:16
%w%f %w%d                                                   -> fibonacci_cache(26) %strace-003.php:16
%w%f %w%d                                                 -> fibonacci_cache(27) %strace-003.php:16
%w%f %w%d                                               -> fibonacci_cache(28) %strace-003.php:16
%w%f %w%d                                             -> fibonacci_cache(29) %strace-003.php:16
%w%f %w%d                                           -> fibonacci_cache(30) %strace-003.php:16
%w%f %w%d                                         -> fibonacci_cache(31) %strace-003.php:16
%w%f %w%d                                       -> fibonacci_cache(32) %strace-003.php:16
%w%f %w%d                                     -> fibonacci_cache(33) %strace-003.php:16
%w%f %w%d                                   -> fibonacci_cache(34) %strace-003.php:16
%w%f %w%d                                 -> fibonacci_cache(35) %strace-003.php:16
%w%f %w%d                               -> fibonacci_cache(36) %strace-003.php:16
%w%f %w%d                             -> fibonacci_cache(37) %strace-003.php:16
%w%f %w%d                           -> fibonacci_cache(38) %strace-003.php:16
%w%f %w%d                         -> fibonacci_cache(39) %strace-003.php:16
%w%f %w%d                       -> fibonacci_cache(40) %strace-003.php:16
%w%f %w%d                     -> fibonacci_cache(41) %strace-003.php:16
%w%f %w%d                   -> fibonacci_cache(42) %strace-003.php:16
%w%f %w%d                 -> fibonacci_cache(43) %strace-003.php:16
%w%f %w%d               -> fibonacci_cache(44) %strace-003.php:16
%w%f %w%d             -> fibonacci_cache(45) %strace-003.php:16
%w%f %w%d           -> fibonacci_cache(46) %strace-003.php:16
%w%f %w%d         -> fibonacci_cache(47) %strace-003.php:16
%w%f %w%d       -> fibonacci_cache(48) %strace-003.php:16
%w%f %w%d     -> xdebug_stop_trace() %strace-003.php:23
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d]
