--TEST--
Filtered tracing: namespace include [3]
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.collect_return=1
xdebug.collect_assignments=0
xdebug.trace_format=0
--FILE--
<?php
$cwd = __DIR__;
xdebug_set_filter(XDEBUG_FILTER_TRACING, XDEBUG_NAMESPACE_INCLUDE, [ '', '\XDEBUG' ] );

include "$cwd/../filter/foobar/foobar.php";
include "$cwd/../filter/xdebug/xdebug.php";

require_once 'capture-trace.inc';

Foobar::foo("hi");
XDEBUG::foo("hi");
Xdebug::foo("hi");
	
xdebug_stop_trace();
?>
--EXPECTF--
ello!
ello!
ello!
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d       -> strstr($haystack = 'Hello!\n', $needle = 'e') %sfilter%efoobar%efoobar.php:6
%w%f %w%d        >=> 'ello!\n'
%w%f %w%d     -> Xdebug::foo($s = 'hi') %strace-filter-ns-include-003.php:11
%w%f %w%d       -> strstr($haystack = 'Hello!\n', $needle = 'e') %sfilter%exdebug%exdebug.php:6
%w%f %w%d        >=> 'ello!\n'
%w%f %w%d     -> Xdebug::foo($s = 'hi') %strace-filter-ns-include-003.php:12
%w%f %w%d       -> strstr($haystack = 'Hello!\n', $needle = 'e') %sfilter%exdebug%exdebug.php:6
%w%f %w%d        >=> 'ello!\n'
%w%f %w%d     -> xdebug_stop_trace() %strace-filter-ns-include-003.php:14
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
