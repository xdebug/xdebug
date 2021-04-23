--TEST--
xdebug_info("mode")
--INI--
display_errors=0
error_log=
xdebug.mode=develop,trace
--FILE--
<?php
var_dump(xdebug_info('mode'));
?>
--EXPECTF--
%sxdebug_info-mode-001.php:2:
array(2) {
  [0] =>
  string(7) "develop"
  [1] =>
  string(5) "trace"
}
