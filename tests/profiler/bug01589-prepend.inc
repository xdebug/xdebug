<?php
function capture()
{
	global $filename;

	echo file_get_contents($filename);
	@unlink($filename);
}

if (strlen('foobar')>1) {
	$foobar=substr('foobar', 1);
}
else {
	$foobar='';
}

$filename = xdebug_get_profiler_filename();
register_shutdown_function('capture');
?>
