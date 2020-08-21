--TEST--
xdebug.mode: Warning for invalid modes
--INI--
display_errors=0
error_log=
xdebug.mode=nonexisting,develop
--FILE--
<?php
?>
--EXPECTF--
Xdebug: [Config] Invalid mode 'nonexisting,develop' set for 'xdebug.mode' configuration setting (See: http%sdocs/errors#CFG-C-MODE)
