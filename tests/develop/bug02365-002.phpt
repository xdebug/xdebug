--TEST--
Test for bug #2365: INI settings error_prepend_string and error_append_string disregarded when a fatal error happens (ansi)
--INI--
xdebug.mode=develop
xdebug.cli_color=2
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
[1m[31mWarning[0m: Undefined variable $undefinedVar[22m in [31m%sbug02365-002.php[0m on line [32m2[0m[22m

[1mCall Stack:[22m
%w%f %w%d   1. {main}() %sbug02365-002.php:0

<!-- APPEND_ERROR_STRING -->


<!-- PREPEND_ERROR_STRING -->
[1m[31mFatal error[0m: Uncaught Error: Class "NotAClass" not found[22m in [31m%sbug02365-002.php[0m on line [32m4[0m[22m

[1m[31mError[0m: Class "NotAClass" not found[22m in [31m%sbug02365-002.php[0m on line [32m4[0m[22m

[1mCall Stack:[22m
%w%f %w%d   1. {main}() %sbug02365-002.php:0

<!-- APPEND_ERROR_STRING -->
