--TEST--
Test for collection errors (2) (fatal error)
--INI--
error_log=NULL
--FILE--
<?php
xdebug_start_error_collection();

trigger_error("An error", E_USER_ERROR);

echo "Errors\n";
var_dump( xdebug_get_collected_errors() );
?>
--EXPECTF--
Fatal error: An error in %serror_collection_2.php on line 4
