<?php
$e = xdebug_get_headers();
xdebug_break();
$e = xdebug_get_headers();
echo "Hi!\n";
?>
