--TEST--
XDEBUG_MODE: Warning for invalid modes
--INI--
display_errors=0
error_log=
xdebug.mode=wrongmode
--ENV--
XDEBUG_MODE=develop,nonexisting
--FILE--
<?php
echo join( ',', xdebug_info( 'mode' ) );
?>
--EXPECTF--
Xdebug: [Config] Invalid mode 'develop,nonexisting' set for 'XDEBUG_MODE' environment variable, fall back to 'xdebug.mode' configuration setting (See: http%sdocs/errors#CFG-C-ENVMODE)
Xdebug: [Config] Invalid mode 'wrongmode' set for 'xdebug.mode' configuration setting (See: http%sdocs/errors#CFG-C-MODE)
Xdebug: [Config] Invalid mode 'develop,nonexisting' set for 'XDEBUG_MODE' environment variable, fall back to 'xdebug.mode' configuration setting (See: http%sdocs/errors#CFG-C-ENVMODE)
develop
