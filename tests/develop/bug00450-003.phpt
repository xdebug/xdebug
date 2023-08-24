--TEST--
Test for bug #450: Incomplete backtraces when an exception gets rethrown (HTML)
--INI--
xdebug.mode=develop
xdebug.dump.GET=
xdebug.dump.SERVER=
xdebug.show_local_vars=0
html_errors=1
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
<br />
<font size='1'><table class='xdebug-error xe-uncaught-exception' dir='ltr' border='1' cellspacing='0' cellpadding='1'>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> Fatal error: Uncaught Exception: foo in %sbug00450-003.php on line <i>18</i></th></tr>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> Exception: foo in %sbug00450-003.php on line <i>18</i></th></tr>
<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Call Stack</th></tr>
<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Memory</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>
<tr><td bgcolor='#eeeeec' align='center'>1</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>{main}()</td><td title='%sbug00450-003.php' bgcolor='#eeeeec'>%sbug00450-003.php<b>:</b>0</td></tr>
<tr><td bgcolor='#eeeeec' align='center'>2</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>test->f4()</td><td title='%sbug00450-003.php' bgcolor='#eeeeec'>%sbug00450-003.php<b>:</b>24</td></tr>
<tr><td bgcolor='#eeeeec' align='center'>3</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>test->f5()</td><td title='%sbug00450-003.php' bgcolor='#eeeeec'>%sbug00450-003.php<b>:</b>7</td></tr>
<tr><td bgcolor='#eeeeec' align='center'>4</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>test->f6()</td><td title='%sbug00450-003.php' bgcolor='#eeeeec'>%sbug00450-003.php<b>:</b>14</td></tr>
</table></font>
