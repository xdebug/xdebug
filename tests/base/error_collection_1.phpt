--TEST--
Test for collection errors (1)
--INI--
display_errors=1
xdebug.default_enable=1
html_errors=0
xdebug.collect_params=4
xdebug.overload_var_dump=0
--FILE--
<?php
xdebug_start_error_collection();

trigger_error("An error", E_USER_WARNING);

echo "Errors\n";
var_dump( xdebug_get_collected_errors() );
?>
--EXPECTF--
Errors
array(1) {
  [0]=>
  string(%d) "
Warning: An error in %serror_collection_1.php on line 4

Call Stack:
%w%f %w%d   1. {main}() %serror_collection_1.php:0
%w%f %w%d   2. trigger_error('An error', 512) %serror_collection_1.php:4

"
}
