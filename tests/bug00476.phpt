--TEST--
Test for bug #476: Exception chanining doesn't work
--SKIPIF--
<?php if(version_compare(phpversion(), "5.3.0", '<')) echo "skip PHP 5.3 needed\n"; ?>
--INI--
xdebug.default_enable=1
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
Fatal error: Uncaught exception 'Exception' with message 'First exception' in %sbug00476.php on line 8

Exception: First exception in %sbug00476.php on line 3

Call Stack:
%w%f %w%d   1. {main}() %sbug00476.php:0

Exception: Second exception in %sbug00476.php on line 6

Call Stack:
%w%f %w%d   1. {main}() %sbug00476.php:0

Exception: Third exception in %sbug00476.php on line 8

Call Stack:
%w%f %w%d   1. {main}() %sbug00476.php:0
