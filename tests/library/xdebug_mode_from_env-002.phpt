--TEST--
XDEBUG_MODE overrides xdebug.mode
--INI--
display_errors=0
error_log=
xdebug.mode=develop
--ENV--
XDEBUG_MODE=develop,profile
--FILE--
<?php
xdebug_info();
?>
--EXPECTF--
%Athrough 'XDEBUG_MODE' env variable%ADevelopment Helpers => ✔ enabled%AProfiler => ✔ enabled%A
