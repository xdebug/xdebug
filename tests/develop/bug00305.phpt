--TEST--
Test for bug #305: xdebug exception handler doesn't properly handle special chars
--INI--
html_errors=1
xdebug.mode=develop
xdebug.file_link_format=xdebug://%f@%l
xdebug.filename_format=
--FILE--
<?php
throw new Exception("<MARK>");
?>
--EXPECTF--
<br />
<font size='1'><table class='xdebug-error xe-uncaught-exception' dir='ltr' border='1' cellspacing='0' cellpadding='1'>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> Fatal error: Uncaught%sException%s&lt;MARK&gt;%sin <a style='color: black' href='xdebug://%sbug00305.php@2'>%sbug00305.php</a> on line <i>2</i></th></tr>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> Exception: &lt;MARK&gt; in <a style='color: black' href='xdebug://%sbug00305.php@2'>%sbug00305.php</a> on line <i>2</i></th></tr>
<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Call Stack</th></tr>
<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Memory</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>
<tr><td bgcolor='#eeeeec' align='center'>1</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>{main}(  )</td><td title='%sbug00305.php' bgcolor='#eeeeec'><a style='color: black' href='xdebug://%sbug00305.php@0'>...%sbug00305.php<b>:</b>0</a></td></tr>
</table></font>
