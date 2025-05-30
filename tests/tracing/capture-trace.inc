<?php
require_once( __DIR__ . '../../utils.inc' );

function show_trace_file() {
	global $tf;

	if (preg_match('@\.gz$@', $tf)) {
		$fp = gzopen($tf, 'r');
		echo stream_get_contents($fp);
	} else {
		echo file_get_contents($tf);
	}
	unlink($tf);
}

register_shutdown_function('show_trace_file');

if (xdebug_get_tracefile_name() === false) {
	$tf = xdebug_start_trace(getTmpFile(uniqid('xdt', TRUE)));
} else {
	$tf = xdebug_get_tracefile_name();
}
?>
