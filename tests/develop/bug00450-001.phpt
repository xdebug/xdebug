--TEST--
Test for bug #450: Incomplete backtraces when an exception gets rethrown (Text)
--INI--
xdebug.mode=develop
xdebug.dump.GET=
xdebug.dump.SERVER=
xdebug.show_local_vars=0
html_errors=0
xdebug.cli_color=0
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
Fatal error: Uncaught Exception: foo in %s450-001.php on line 18

Exception: foo in %s450-001.php on line 18

Call Stack:
%w%f %w%d   1. {main}() %s450-001.php:0
%w%f %w%d   2. test->f4() %s450-001.php:24
%w%f %w%d   3. test->f5() %s450-001.php:7
%w%f %w%d   4. test->f6() %s450-001.php:14
