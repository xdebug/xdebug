--TEST--
Filtered tracing: path include [1]
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.collect_return=1
xdebug.collect_assignments=0
xdebug.trace_format=0
--FILE--
<?php
$cwd = __DIR__; $s = DIRECTORY_SEPARATOR; $includeDir = realpath( $cwd . '/..' );
xdebug_set_filter(XDEBUG_FILTER_TRACING, XDEBUG_PATH_INCLUDE, [ "{$includeDir}{$s}filter{$s}xdebug" ] );

include "{$includeDir}/filter/foobar/foobar.php";
include "{$includeDir}/filter/xdebug/xdebug.php";

require_once 'capture-trace.inc';

Foobar::foo("hi");
Xdebug::foo("hi");
	
xdebug_stop_trace();
?>
--EXPECTF--
ello!
ello!
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d       -> strstr($haystack = 'Hello!\n', $needle = 'e') %sfilter%exdebug%exdebug.php:6
%w%f %w%d        >=> 'ello!\n'
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
