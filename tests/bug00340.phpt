--TEST--
Test for bug #340: Segfault while throwing an Exception
--INI--
xdebug.default_enable=1
html_errors=1
xdebug.file_link_format=
xdebug.dump.GET=
xdebug.dump.SERVER=
--FILE--
<?php
class MyException extends Exception
{
	function __construct()
	{
		$this->message = new StdClass;
	}
}


throw new MyException();
?>
--EXPECTF--
<br />
<font size='1'><table class='xdebug-error' dir='ltr' border='1' cellspacing='0' cellpadding='1'>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> Fatal error: Your exception class uses incorrect types for common properties: 'message' and 'file' need to be a string and 'line' needs to be an integer. in %sbug00340.php on line <i>11</i></th></tr>
<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Call Stack</th></tr>
<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Memory</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>
<tr><td bgcolor='#eeeeec' align='center'>1</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>{main}(  )</td><td title='%sbug00340.php' bgcolor='#eeeeec'>../bug00340.php<b>:</b>0</td></tr>
</table></font>
