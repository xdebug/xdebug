--TEST--
Filtered tracing: path whitelist [4]
--INI--
xdebug.auto_trace=0
xdebug.collect_return=1
xdebug.collect_params=4
xdebug.collect_assignments=0
xdebug.trace_format=0
--FILE--
<?php
$cwd = __DIR__; $s = DIRECTORY_SEPARATOR;
xdebug_set_filter(XDEBUG_FILTER_TRACING, XDEBUG_PATH_WHITELIST, [] );

include "$cwd/filter/foobar/foobar.php";
include "$cwd/filter/xdebug/xdebug.php";

$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

Foobar::foo("hi");
Xdebug::foo("hi");

xdebug_set_filter(XDEBUG_FILTER_TRACING, XDEBUG_PATH_WHITELIST, [ "{$cwd}{$s}filter{$s}foobar", "" ] );

Foobar::foo("hi");
Xdebug::foo("hi");

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
ello!
ello!
ello!
ello!
TRACE START [%d-%d-%d %d:%d:%d]
%w%f %w%d     -> Foobar::foo($s = 'hi') %strace-filter-path-white-4.php:15
%w%f %w%d       -> strstr('Hello!\n', 'e') %sfilter%efoobar%efoobar.php:6
%w%f %w%d        >=> 'ello!\n'
%w%f %w%d     -> Xdebug::foo($s = 'hi') %strace-filter-path-white-4.php:16
%w%f %w%d       -> strstr('Hello!\n', 'e') %sfilter%exdebug%exdebug.php:6
%w%f %w%d        >=> 'ello!\n'
%w%f %w%d     -> xdebug_stop_trace() %strace-filter-path-white-4.php:18
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d]
