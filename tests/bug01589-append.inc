<?php
if (strlen('foobar')>1) {
	$foobar=substr('foobar', 1);
}
else {
	$foobar='';
}

$filename = xdebug_get_profiler_filename();
?>
