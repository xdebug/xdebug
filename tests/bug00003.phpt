--TEST--
Text for crash bug in tracing to file
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
--FILE--
<?php
	@unlink('/tmp/bug00003.trace');
	xdebug_start_trace('/tmp/bug00003.trace');
	strftime('%b %l %Y %H:%M:%S', 1061728888);
	xdebug_stop_trace();
	readfile('/tmp/bug00003.trace');
	unlink('/tmp/bug00003.trace');
?>
--EXPECTF--

Start of function trace
    %f      %d     -> strftime('%b %l %Y %H:%M:%S', 1061728888) /%s/bug00003.php:4
    %f      %d     -> xdebug_stop_trace() /%s/bug00003.php:5
End of function trace
