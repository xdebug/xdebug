--TEST--
xdebug_info("mode") with xdebug.mode=off, overridden by XDEBUG_MODE
--INI--
display_errors=0
error_log=
xdebug.mode=off
--ENV--
XDEBUG_MODE=gcstats,coverage
--FILE--
<?php
var_dump(xdebug_info('mode'));
?>
--EXPECTF--
array(2) {
  [0]=>
  string(8) "coverage"
  [1]=>
  string(7) "gcstats"
}
