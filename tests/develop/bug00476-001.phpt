--TEST--
Test for bug #476: Exception chanining doesn't work (Text)
--INI--
xdebug.mode=develop
xdebug.dump.GET=
xdebug.dump.SERVER=
xdebug.show_local_vars=0
html_errors=0
xdebug.cli_color=0
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
?>
--FILE--
<?php
function throwSecondException( Exception $e )
{
   throw new Exception('Second exception', 0, $e);
}

class thrower
{
	function __construct( private Exception $e )
	{
	}

	function throwMe()
	{
		throw new Exception('Third exception', 42, $this->e );
	}
}

try {
   throw new Exception('First exception');
} catch(Exception $e) {
	try {
		throwSecondException( $e );
	} catch(Exception $f) {
		$t = new thrower($f);
		$t->throwMe();
	}
}
echo "DONE\n";
?>
--EXPECTF--
Fatal error: Uncaught Exception: First exception in %sbug00476-001.php:20
Stack trace:
#0 {main}

Next Exception: Second exception in %sbug00476-001.php:4
Stack trace:
#0 %sbug00476-001.php(23): throwSecondException(Object(Exception))
#1 {main}

Next Exception: Third exception in %sbug00476-001.php on line 15

Exception: Third exception in %sbug00476-001.php on line 15

Call Stack:
%w%f %w%d   1. {main}() %sbug00476-001.php:0
%w%f %w%d   2. thrower->throwMe() %sbug00476-001.php:26

	Nested Exceptions:

	Exception: Second exception in %sbug00476-001.php on line 4

	Call Stack:
	%w%f %w%d   1. {main}() %sbug00476-001.php:0
	%w%f %w%d   2. throwSecondException() %sbug00476-001.php:23

	Exception: First exception in %sbug00476-001.php on line 20

	Call Stack:
	%w%f %w%d   1. {main}() %sbug00476-001.php:0
