--TEST--
xdebug_info("mode") overriden by XDEBUG_MODE
--INI--
display_errors=0
error_log=
xdebug.mode=develop,trace
--ENV--
XDEBUG_MODE=profile,debug
--FILE--
<?php
var_dump(xdebug_info('mode'));
?>
--EXPECTF--
array(2) {
  [0]=>
  string(5) "debug"
  [1]=>
  string(7) "profile"
}
