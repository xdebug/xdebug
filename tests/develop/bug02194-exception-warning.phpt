--TEST--
Test for bug #2194: xdebug_get_function_stack(['from_exception']) warnings with incompatible options
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('!opcache');
?>
--INI--
xdebug.mode=develop
xdebug.auto_profile=0
xdebug.var_display_max_depth=4
--FILE--
<?php
$e = new Exception();

xdebug_get_function_stack( [ 'from_exception' => $e, 'local_vars' => true ] );
xdebug_get_function_stack( [ 'from_exception' => $e, 'params_as_values' => true ] );

?>
--EXPECTF--
Warning: The 'local_vars' or 'params_as_values' options are ignored when used with the 'from_exception' option in %sbug02194-exception-warning.php on line 4

Call Stack:%A

Warning: The 'local_vars' or 'params_as_values' options are ignored when used with the 'from_exception' option in %sbug02194-exception-warning.php on line 5

Call Stack:%A
