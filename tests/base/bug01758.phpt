--TEST--
Test for bug #1758: Replacing error_get_last data when it shouldn't
--INI--
xdebug.default_enable=1
--FILE--
<?php
register_shutdown_function(function () {
    echo is_null(error_get_last()) ? 'null' : 'not null';
});

try {
    new DateTime('Incorrect value');
} catch (Exception $e) { }
--EXPECTF--
null
