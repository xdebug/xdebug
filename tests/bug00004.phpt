--TEST--
Text for crash bug in dumping a trace
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
--FILE--
<?php
	xdebug_start_trace();
	strftime('%b %l %Y %H:%M:%S', 1061728888);
	xdebug_dump_function_trace();
?>
--EXPECTF--

Function trace:
    %f      %d     -> strftime('%b %l %Y %H:%M:%S', 1061728888) /%s/bug00004.php:3
