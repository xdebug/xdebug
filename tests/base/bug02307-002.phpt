--TEST--
Test for bug #2307: Segmentation fault due to a superglobal being a reference [2]
--INI--
xdebug.mode=develop,trace
--FILE--
<?php
echo "bla";
$fasel = &$_GET;

set_exception_handler(static function () {
echo '1';
});

require_once '_non_existing_file';
?>
--EXPECTF--
bla
Warning: require_once(_non_existing_file): Failed to open stream: No such file or directory in %sbug02307-002.php on line %d
%A
1
