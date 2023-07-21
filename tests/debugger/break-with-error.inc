<?php
$e = xdebug_get_headers();
@trigger_error('foo');
$e = xdebug_get_headers();
echo "Not triggered!\n";
?>
