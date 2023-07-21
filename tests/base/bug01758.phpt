--TEST--
Test for bug #1758: Xdebug changes error_get_last results inside a try catch
--INI--
xdebug.mode=develop
--FILE--
<?php
register_shutdown_function(function () {
    var_dump(error_get_last());
});

try {
    new DateTime('Incorrect value');
} catch (Exception $e) {
}
?>
--EXPECTF--
%sbug01758.php:%d:
NULL
