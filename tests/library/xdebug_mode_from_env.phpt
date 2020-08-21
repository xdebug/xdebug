--TEST--
xdebug.mode: Warning for invalid modes
--INI--
display_errors=0
error_log=
--ENV--
XDEBUG_MODE=develop,nonexisting
--FILE--
<?php
?>
--EXPECTF--
Xdebug: [Config] Invalid mode 'develop,nonexisting' set for 'XDEBUG_MODE' environment variable (See: http%sdocs/errors#CFG-C-ENVMODE)
Xdebug: [Config] Invalid mode 'develop,nonexisting' set for 'XDEBUG_MODE' environment variable (See: http%sdocs/errors#CFG-C-ENVMODE)
