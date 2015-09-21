--TEST--
Test for bug #476: Exception chanining doesn't work (< PHP 7.0)
--SKIPIF--
<?php if (!version_compare(phpversion(), "7.0", '<')) echo "skip < PHP 7.0 needed\n"; ?>
--INI--
xdebug.default_enable=1
xdebug.dump.GET=
xdebug.dump.SERVER=
xdebug.show_local_vars=0
--FILE--
<?php

function a()
{
   throw new Exception('First exception');
}

function b()
{
	try {
		a();
	} catch(Exception $e) {
	   throw new Exception('Second exception', 0, $e);
	}
}

function c()
{
	try {
		b();
	} catch(Exception $e) {
	   throw new Exception('Third exception', 0, $e);
	}
}

function d()
{
	try {
		c();
	} catch(Exception $e) {
	   throw new Exception('Fourth exception', 0, $e);
	}
}

d();

echo "DONE\n";
?>
--EXPECTF--
Fatal error: Uncaught exception 'Exception' with message 'First exception' in %sbug00476-002-php5.php on line 31

Exception: First exception in %sbug00476-002-php5.php on line 5

Call Stack:
%w%f %w%d   1. {main}() %sbug00476-002-php5.php:0
%w%f %w%d   2. d() %sbug00476-002-php5.php:35
%w%f %w%d   3. c() %sbug00476-002-php5.php:29
%w%f %w%d   4. b() %sbug00476-002-php5.php:20
%w%f %w%d   5. a() %sbug00476-002-php5.php:11

Exception: Second exception in %sbug00476-002-php5.php on line 13

Call Stack:
%w%f %w%d   1. {main}() %sbug00476-002-php5.php:0
%w%f %w%d   2. d() %sbug00476-002-php5.php:35
%w%f %w%d   3. c() %sbug00476-002-php5.php:29
%w%f %w%d   4. b() %sbug00476-002-php5.php:20

Exception: Third exception in %sbug00476-002-php5.php on line 22

Call Stack:
%w%f %w%d   1. {main}() %sbug00476-002-php5.php:0
%w%f %w%d   2. d() %sbug00476-002-php5.php:35
%w%f %w%d   3. c() %sbug00476-002-php5.php:29

Exception: Fourth exception in %sbug00476-002-php5.php on line 31

Call Stack:
%w%f %w%d   1. {main}() %sbug00476-002-php5.php:0
%w%f %w%d   2. d() %sbug00476-002-php5.php:35
