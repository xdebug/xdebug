--TEST--
Test for bug #178: $php_errormsg and Track errors unavailable
--INI--
xdebug.default_enable=1
xdebug.auto_trace=0
xdebug.trace_options=0
xdebug.trace_output_dir=/tmp
xdebug.collect_return=1
xdebug.collect_params=1
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.dump_globals=0
xdebug.show_mem_delta=0
xdebug.trace_format=0
track_errors=1
display_errors=0
xdebug.force_display_errors=0
--FILE--
<?php
fsockopen();
echo "> ", $php_errormsg, "\n";
fsockopen( 'localhost', 5000, $errno, $errstr, 0.02 );
echo "> ", str_replace( [ "\n", "\r" ], [ '', '' ], $php_errormsg ), "\n";
?>
DONE
--EXPECTF--
> fsockopen() expects at least 1 parameter, 0 given
> %snable to connect to localhost:500%s
DONE
