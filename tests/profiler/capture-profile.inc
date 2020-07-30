<?php
$filename = xdebug_get_profiler_filename();

function capture() {
	global $filename;
	echo file_get_contents($filename);
	unlink($filename);
	@unlink(xdebug_get_profiler_filename());
}
register_shutdown_function('capture');
?>
