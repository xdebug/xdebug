--TEST--
Test for bug #340: Segfault while throwing an Exception
--INI--
xdebug.mode=develop
html_errors=1
xdebug.file_link_format=[[%f:%l]]
xdebug.filename_format=
xdebug.dump.GET=
xdebug.dump.SERVER=
--FILE--
<?php
class MyException extends Exception
{
	function __construct()
	{
		$this->message = array(1, 2, 3);
	}
}


throw new MyException();
?>
--EXPECTF--
<br />
<font size='1'><table class='xdebug-error xe-%s' dir='ltr' border='1' cellspacing='0' cellpadding='1'>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> %s: Array to string conversion in Unknown on line <i>0</i></th></tr>
<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Call Stack</th></tr>
<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Memory</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>
<tr><td bgcolor='#eeeeec' align='center'>1</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'><a href='http://www.php.net/MyException.--toString.html' target='_new'>__toString</a>(  )</td><td title='Unknown' bgcolor='#eeeeec'>...%sUnknown<b>:</b>0</td></tr>
</table></font>
<br />
<font size='1'><table class='xdebug-error xe-uncaught-exception' dir='ltr' border='1' cellspacing='0' cellpadding='1'>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> Fatal error: Uncaught%sMyException%sArray%sin <a style='color: black' href='[[%sbug00340-002.php:11]]'>%sbug00340-002.php</a> on line <i>11</i></th></tr>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> MyException:  in <a style='color: black' href='[[%sbug00340-002.php:11]]'>%sbug00340-002.php</a> on line <i>11</i></th></tr>
<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Call Stack</th></tr>
<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Memory</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>
<tr><td bgcolor='#eeeeec' align='center'>1</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>{main}(  )</td><td title='%sbug00340-002.php' bgcolor='#eeeeec'><a style='color: black' href='[[%sbug00340-002.php:0]]'>...%sbug00340-002.php<b>:</b>0</a></td></tr>
</table></font>
