--TEST--
xdebug_info("mode") overridden by XDEBUG_MODE=off
--INI--
display_errors=0
error_log=
xdebug.mode=gcstats,develop
--ENV--
XDEBUG_MODE=off
--FILE--
<?php
var_dump(xdebug_info('mode'));
?>
--EXPECTF--
array(0) {
}
