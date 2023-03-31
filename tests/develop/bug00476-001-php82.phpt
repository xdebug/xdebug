--TEST--
Test for bug #476: Exception chanining doesn't work (>= PHP 8.2)
--INI--
xdebug.mode=develop
xdebug.dump.GET=
xdebug.dump.SERVER=
xdebug.show_local_vars=0
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.2');
?>
--FILE--
<?php
try {
   throw new Exception('First exception');
} catch(Exception $e) {
	try {
	   throw new Exception('Second exception', 0, $e);
	} catch(Exception $f) {
	   throw new Exception('Third exception', 0, $f);
	}
}
echo "DONE\n";
?>
--EXPECTF--
Fatal error: Uncaught Exception: First exception in %sbug00476-001-php82.php:3
Stack trace:
#0 {main}

Next Exception: Second exception in %sbug00476-001-php82.php:6
Stack trace:
#0 {main}

Next Exception: Third exception in %sbug00476-001-php82.php on line 8

Exception: Third exception in %sbug00476-001-php82.php on line 8

Call Stack:
%w%f %w%d   1. {main}() %sbug00476-001-php82.php:0
