--TEST--
Test for bug #476: Exception chanining doesn't work
--INI--
xdebug.mode=develop
xdebug.dump.GET=
xdebug.dump.SERVER=
xdebug.show_local_vars=0
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
Fatal error: Uncaught Exception: First exception in %sbug00476-001.php:3
Stack trace:
#0 {main}

Next Exception: Second exception in %sbug00476-001.php:6
Stack trace:
#0 {main}

Next Exception: Third exception in %sbug00476-001.php on line 8

Exception: First exception in %sbug00476-001.php on line 3

Call Stack:
%w%f %w%d   1. {main}() %sbug00476-001.php:0

Exception: Second exception in %sbug00476-001.php on line 6

Call Stack:
%w%f %w%d   1. {main}() %sbug00476-001.php:0

Exception: Third exception in %sbug00476-001.php on line 8

Call Stack:
%w%f %w%d   1. {main}() %sbug00476-001.php:0
