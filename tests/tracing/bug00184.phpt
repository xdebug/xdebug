--TEST--
Test for bug #184: problem with control chars in code traces
--INI--
xdebug.mode=trace
xdebug.start_with_request=yes
xdebug.trace_options=0
xdebug.trace_output_name=trace.%c
xdebug.collect_return=1
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.dump_globals=0
xdebug.trace_format=0
xdebug.var_display_max_depth=2
xdebug.var_display_max_children=33
--FILE--
<?php
	$trace_file = xdebug_get_tracefile_name();
	str_replace(range("\0", " "), ' ', 'foobar');
	xdebug_stop_trace();
	echo file_get_contents($trace_file);
	unlink($trace_file);
	echo "DONE\n";
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d   -> {main}() %sbug00184.php:0
%w%f %w%d     -> xdebug_get_tracefile_name() %sbug00184.php:2
%w%f %w%d      >=> '%strace.%d.xt'
%w%f %w%d     -> range($%s = '\000', $%s = ' ') %sbug00184.php:3
%w%f %w%d      >=> [0 => '\000', 1 => '\001', 2 => '\002', 3 => '\003', 4 => '\004', 5 => '\005', 6 => '\006', 7 => '\a', 8 => '\b', 9 => '\t', 10 => '\n', 11 => '\v', 12 => '\f', 13 => '\r', 14 => '\016', 15 => '\017', 16 => '\020', 17 => '\021', 18 => '\022', 19 => '\023', 20 => '\024', 21 => '\025', 22 => '\026', 23 => '\027', 24 => '\030', 25 => '\031', 26 => '\032', 27 => '\033', 28 => '\034', 29 => '\035', 30 => '\036', 31 => '\037', 32 => ' ']
%w%f %w%d     -> str_replace($search = [0 => '\000', 1 => '\001', 2 => '\002', 3 => '\003', 4 => '\004', 5 => '\005', 6 => '\006', 7 => '\a', 8 => '\b', 9 => '\t', 10 => '\n', 11 => '\v', 12 => '\f', 13 => '\r', 14 => '\016', 15 => '\017', 16 => '\020', 17 => '\021', 18 => '\022', 19 => '\023', 20 => '\024', 21 => '\025', 22 => '\026', 23 => '\027', 24 => '\030', 25 => '\031', 26 => '\032', 27 => '\033', 28 => '\034', 29 => '\035', 30 => '\036', 31 => '\037', 32 => ' '], $replace = ' ', $subject = 'foobar') %sbug00184.php:3
%w%f %w%d      >=> 'foobar'
%w%f %w%d     -> xdebug_stop_trace() %sbug00184.php:4
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]

DONE
