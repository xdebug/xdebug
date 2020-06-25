--TEST--
Test for bug #389: Destructors called on fatal error (PHP < 8.0)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP < 8.0');
?>
--INI--
log_errors=0
xdebug.mode=develop
xdebug.dump.GET=
xdebug.dump.SERVER=
xdebug.show_local_vars=0
xdebug.show_error_trace=1
--FILE--
<?php
class Food {
    public function __destruct() {
        echo "__destruct called\n";
    }
  }

$f = new Food();
echo "forcing fatal error:\n";
Abc::def();

?>
DONE
--EXPECTF--
forcing fatal error:


Error: Class 'Abc' not found in %sbug00389.php on line 10

Call Stack:
%w%f%w%d   1. {main}() %sbug00389.php:0


Fatal error: Uncaught Error: Class 'Abc' not found in %sbug00389.php on line 10

Error: Class 'Abc' not found in %sbug00389.php on line 10

Call Stack:
%w%f%w%d   1. {main}() %sbug00389.php:0
