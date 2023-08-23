--TEST--
Test for bug #476: Exception chanining doesn't work (CLI colour)
--INI--
xdebug.mode=develop
xdebug.dump.GET=
xdebug.dump.SERVER=
xdebug.show_local_vars=0
html_errors=0
xdebug.cli_color=2
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
[1m[31mFatal error[0m: Uncaught Exception: First exception in %sbug00476-002.php:20
Stack trace:
#0 {main}

Next Exception: Second exception in %sbug00476-002.php:4
Stack trace:
#0 %sbug00476-002.php(23): throwSecondException(Object(Exception))
#1 {main}

Next Exception: Third exception[22m in [31m%sbug00476-002.php[0m on line [32m15[0m[22m

[1m[31mException[0m: Third exception[22m in [31m%sbug00476-002.php[0m on line [32m15[0m[22m

[1mCall Stack:[22m
%w%f %w%d   1. {main}() %sbug00476-002.php:0
%w%f %w%d   2. thrower->throwMe() %sbug00476-002.php:26

	[1mNested Exceptions:[22m

	[1m[31mException[0m: Second exception[22m in [31m%sbug00476-002.php[0m on line [32m4[0m[22m

	[1mCall Stack:[22m
	%w%f %w%d   1. {main}() %sbug00476-002.php:0
	%w%f %w%d   2. throwSecondException() %sbug00476-002.php:23

	[1m[31mException[0m: First exception[22m in [31m%sbug00476-002.php[0m on line [32m20[0m[22m

	[1mCall Stack:[22m
	%w%f %w%d   1. {main}() %sbug00476-002.php:0
