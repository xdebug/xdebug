--TEST--
Test for bug #2427: Crash when file_link_format setting is wrong
--INI--
xdebug.mode=develop
html_errors=On
xdebug.file_link_format=%
--FILE--
<?php
trigger_error('boom');
?>
--EXPECTF--
<br />
<font size='1'><table class='xdebug-error xe-notice' dir='ltr' border='1' cellspacing='0' cellpadding='1'>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> Notice: boom in <a style='color: black' href='%'>%sbug02427-file-link-format.php</a> on line <i>2</i></th></tr>
<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Call Stack</th></tr>
<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Memory</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>
<tr><td bgcolor='#eeeeec' align='center'>1</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>{main}(  )</td><td title='%sbug02427-file-link-format.php' bgcolor='#eeeeec'><a style='color: black' href='%'>...%sbug02427-file-link-format.php<b>:</b>0</a></td></tr>
<tr><td bgcolor='#eeeeec' align='center'>2</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'><a href='http://www.php.net/function.trigger-error.html' target='_new'>trigger_error</a>( <span>$message = </span><span>&#39;boom&#39;</span> )</td><td title='%sbug02427-file-link-format.php' bgcolor='#eeeeec'><a style='color: black' href='%'>...%sbug02427-file-link-format.php<b>:</b>2</a></td></tr>
</table></font>
