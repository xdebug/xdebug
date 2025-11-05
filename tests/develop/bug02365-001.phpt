--TEST--
Test for bug #2365: INI settings error_prepend_string and error_append_string disregarded when a fatal error happens (text)
--INI--
xdebug.mode=develop
error_prepend_string="<!-- PREPEND_ERROR_STRING -->"
error_append_string="<!-- APPEND_ERROR_STRING -->"
--FILE--
<?php
$a = $undefinedVar;
echo "\n\n\n";
new NotAClass();
?>
--EXPECTF--
<!-- PREPEND_ERROR_STRING -->
Warning: Undefined variable $undefinedVar in %sbug02365-001.php on line 2

Call Stack:
%w%f %w%d   1. {main}() %sbug02365-001.php:0

<!-- APPEND_ERROR_STRING -->


<!-- PREPEND_ERROR_STRING -->
Fatal error: Uncaught Error: Class "NotAClass" not found in %sbug02365-001.php on line 4

Error: Class "NotAClass" not found in %sbug02365-001.php on line 4

Call Stack:
%w%f %w%d   1. {main}() %sbug02365-001.php:0

<!-- APPEND_ERROR_STRING -->
