<?php
$filename = xdebug_get_profiler_filename();

function capture() {
	global $filename;

	if (preg_match('@\.gz$@', $filename)) {
		$fp = gzopen($filename, 'r');
		echo stream_get_contents($fp);
	} else {
		echo file_get_contents($filename);
	}
	unlink($filename);
	@unlink(xdebug_get_profiler_filename());
}
register_shutdown_function('capture');
?>
