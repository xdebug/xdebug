--TEST--
xdebug.mode: Warning for invalid modes
--INI--
display_errors=0
error_log=
xdebug.mode=develop,nonexisting
--FILE--
<?php
?>
--EXPECTF--
Xdebug: [Config] Invalid mode 'develop,nonexisting' set for 'xdebug.mode' configuration setting (See: http%sdocs/errors#CFG-C-MODE)
