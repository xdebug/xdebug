--TEST--
Test for bug #1758: Xdebug changes error_get_last results inside a try catch (< PHP 7.3)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP < 7.3');
?>
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
array(4) {
  ["type"]=>
  int(%d)
  ["message"]=>
  string(%d) "%s"
  ["file"]=>
  string(%d) "%s"
  ["line"]=>
  int(%d)
}
