--TEST--
Test for bug #2321: Seg fault when null is assigned to $_COOKIE
--INI--
xdebug.mode=develop,trace
--FILE--
<?php
echo "bla";
$_COOKIE = NULL;

set_exception_handler(static function () {
echo '1';
});

require_once '_non_existing_file';
?>
--EXPECTF--
bla
Warning: require_once(_non_existing_file): Failed to open stream: No such file or directory in %sbug02321-003.php on line %d
%A
1
