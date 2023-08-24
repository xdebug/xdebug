--TEST--
Test for bug #450: Incomplete backtraces when an exception gets rethrown (Ansi)
--INI--
xdebug.mode=develop
xdebug.dump.GET=
xdebug.dump.SERVER=
xdebug.show_local_vars=0
html_errors=0
xdebug.cli_color=2
display_errors=1
error_reporting=E_ALL
--FILE--
<?php

class test
{
	function f4() {
		try {
			$this->f5();
		} catch(exception $e) {
			throw $e;
		}
	}

	function f5() {
		$this->f6();
	}

	function f6() {
		throw new exception('foo');
	}
}

$test = new test();

$test->f4();

?>
--EXPECTF--
[1m[31mFatal error[0m: Uncaught Exception: foo[22m in [31m%sbug00450-002.php[0m on line [32m18[0m[22m

[1m[31mException[0m: foo[22m in [31m%sbug00450-002.php[0m on line [32m18[0m[22m

[1mCall Stack:[22m
%w%f %w%d   1. {main}() %sbug00450-002.php:0
%w%f %w%d   2. test->f4() %sbug00450-002.php:24
%w%f %w%d   3. test->f5() %sbug00450-002.php:7
%w%f %w%d   4. test->f6() %sbug00450-002.php:14
