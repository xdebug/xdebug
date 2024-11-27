--TEST--
Test for bug #2305: Segfault when checking whether to ignore creating a debug connection during shutdown functions
--INI--
xdebug.discover_client_host=true
xdebug.mode=debug
xdebug.start_with_request=trigger
--FILE--
<?php
register_shutdown_function(function () {});
parse_str("", $_POST);
?>
--EXPECTF--
