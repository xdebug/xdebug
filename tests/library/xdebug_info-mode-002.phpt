--TEST--
xdebug_info("mode") with xdebug.mode=off
--INI--
display_errors=0
error_log=
xdebug.mode=off
--FILE--
<?php
var_dump(xdebug_info('mode'));
?>
--EXPECTF--
array(0) {
}
