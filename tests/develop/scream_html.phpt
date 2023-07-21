--TEST--
Test for scream support (HTML)
--INI--
xdebug.mode=develop
xdebug.scream=0
error_reporting(E_ALL);
html_errors=1
xdebug.file_link_format=xdebug://%f@%l
xdebug.filename_format=
xdebug.force_error_reporting=0
--FILE--
<?php
echo @hex2bin('4'), "\n";
ini_set('xdebug.scream', 1);
echo @hex2bin('4'), "\n";
ini_set('xdebug.scream', 0);
echo @hex2bin('4'), "\n";
?>
--EXPECTF--
<br />
<font size='1'><table class='xdebug-error xe-warning xe-scream' dir='ltr' border='1' cellspacing='0' cellpadding='1'>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> SCREAM: Error suppression ignored for</th></tr>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> Warning: %s in <a style='color: black' href='xdebug://%sscream_html.php@4'>%sscream_html.php</a> on line <i>4</i></th></tr>
<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Call Stack</th></tr>
<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Memory</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>
<tr><td bgcolor='#eeeeec' align='center'>1</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>{main}(  )</td><td title='%sscream_html.php' bgcolor='#eeeeec'><a style='color: black' href='xdebug://%sscream_html.php@0'>...%sscream_html.php<b>:</b>0</a></td></tr>
<tr><td bgcolor='#eeeeec' align='center'>2</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'><a href='http://www.php.net/function.hex2bin.html' target='_new'>hex2bin</a>( <span>$%s = </span><span>&#39;4&#39;</span> )</td><td title='%sscream_html.php' bgcolor='#eeeeec'><a style='color: black' href='xdebug://%sscream_html.php@4'>...%sscream_html.php<b>:</b>4</a></td></tr>
</table></font>
