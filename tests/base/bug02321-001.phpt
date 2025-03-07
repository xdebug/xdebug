--TEST--
Test for bug #2321: Seg fault when null is assigned to $_POST
--INI--
xdebug.mode=develop,trace
--FILE--
<?php
echo "bla";
$_POST = NULL;

set_exception_handler(static function () {
echo '1';
});

require_once '_non_existing_file';
?>
--EXPECTF--
bla
Warning: require_once(_non_existing_file): Failed to open stream: No such file or directory in %sbug02321-001.php on line %d
%A
1
