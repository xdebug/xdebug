--TEST--
Test for bug #2365: INI settings error_prepend_string and error_append_string disregarded when a fatal error happens (HTML)
--INI--
xdebug.mode=develop
html_errors=1
error_prepend_string="<!-- PREPEND_ERROR_STRING -->"
error_append_string="<!-- APPEND_ERROR_STRING -->"
--FILE--
<?php
$a = $undefinedVar;
echo "\n\n\n";
new NotAClass();
?>
--EXPECTF--
<!-- PREPEND_ERROR_STRING --><br />
<font size='1'><table class='xdebug-error xe-warning' dir='ltr' border='1' cellspacing='0' cellpadding='1'>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> Warning: Undefined variable $undefinedVar in %sbug02365-003.php on line <i>2</i></th></tr>
<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Call Stack</th></tr>
<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Memory</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>
<tr><td bgcolor='#eeeeec' align='center'>1</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>{main}(  )</td><td title='%sbug02365-003.php' bgcolor='#eeeeec'>%sbug02365-003.php<b>:</b>0</td></tr>
</table></font>
<!-- APPEND_ERROR_STRING -->


<!-- PREPEND_ERROR_STRING --><br />
<font size='1'><table class='xdebug-error xe-uncaught-exception' dir='ltr' border='1' cellspacing='0' cellpadding='1'>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> Fatal error: Uncaught Error: Class "NotAClass" not found in %sbug02365-003.php on line <i>4</i></th></tr>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> Error: Class "NotAClass" not found in %sbug02365-003.php on line <i>4</i></th></tr>
<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Call Stack</th></tr>
<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Memory</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>
<tr><td bgcolor='#eeeeec' align='center'>1</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>{main}(  )</td><td title='%sbug02365-003.php' bgcolor='#eeeeec'>%sbug02365-003.php<b>:</b>0</td></tr>
</table></font>
<!-- APPEND_ERROR_STRING -->
