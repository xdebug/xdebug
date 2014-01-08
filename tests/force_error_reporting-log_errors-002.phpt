--TEST--
Test for xdebug.force_error_reporting (log_errors) [2]
--INI--
xdebug.default_enable=1
display_errors=0
log_errors=1
xdebug.force_display_errors=0
xdebug.force_error_reporting=E_USER_WARNING|E_WARNING
xdebug.collect_params=0
--FILE--
<?php
ini_set('error_reporting', 0);

trigger_error('four', E_USER_WARNING);
trigger_error('five');
strlen();
?>
--EXPECTF--
PHP Warning:  four in %sforce_error_reporting-log_errors-002.php on line 4
PHP Stack trace:
PHP   1. {main}() %sforce_error_reporting-log_errors-002.php:0
PHP   2. trigger_error() %sforce_error_reporting-log_errors-002.php:4
PHP Warning:  %s in %sforce_error_reporting-log_errors-002.php on line 6
PHP Stack trace:
PHP   1. {main}() %sforce_error_reporting-log_errors-002.php:0
PHP   2. strlen() %sforce_error_reporting-log_errors-002.php:6
