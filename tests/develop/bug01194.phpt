--TEST--
Test for bug #1194: The error message is doubly HTML-encoded with assert()
--INI--
assert.exception=0
error_reporting=E_ALL & ~E_DEPRECATED
html_errors=1
xdebug.mode=develop
xdebug.file_link_format=
xdebug.filename_format=
--FILE--
<?php
assert(0, "TEST&TEST");
?>
--EXPECTF--
<br />
<font size='1'><table class='xdebug-error xe-warning' dir='ltr' border='1' cellspacing='0' cellpadding='1'>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> Warning: assert(): TEST&amp;TEST failed in %sbug01194.php on line <i>2</i></th></tr>
<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Call Stack</th></tr>
<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Memory</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>
<tr><td bgcolor='#eeeeec' align='center'>1</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>{main}(  )</td><td title='%sbug01194.php' bgcolor='#eeeeec'>...%sbug01194.php<b>:</b>0</td></tr>
<tr><td bgcolor='#eeeeec' align='center'>2</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'><a href='http://www.php.net/function.assert.html' target='_new'>assert</a>( <span>$assertion = </span><span>%r(0|FALSE)%r</span>, <span>$description = </span><span>&#39;TEST&amp;TEST&#39;</span> )</td><td title='%sbug01194.php' bgcolor='#eeeeec'>...%sbug01194.php<b>:</b>2</td></tr>
</table></font>
