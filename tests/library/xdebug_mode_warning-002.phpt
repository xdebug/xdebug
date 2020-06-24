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
%sInvalid mode 'nonexisting,develop' set for 'xdebug.mode' configuration setting (See: https://xdebug.org/docs/all_settings#mode)%s
