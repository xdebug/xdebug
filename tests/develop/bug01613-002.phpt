--TEST--
Test for bug #1613: Wrong name displayed for Recoverable fatal error [html] (< PHP 7.4)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP < 7.4');
?>
--INI--
xdebug.mode=develop
xdebug.collect_assignments=0
xdebug.show_local_vars=0
xdebug.dump_globals=0
html_errors=1
--FILE--
<?php
$v = new DateTime();
$v = (string) $v;
?>
--EXPECTF--
<br />
<font size='1'><table class='xdebug-error xe-recoverable-fatal-error' dir='ltr' border='1' cellspacing='0' cellpadding='1'>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> Recoverable fatal error: Object of class DateTime could not be converted to string in %sbug01613-002.php on line <i>3</i></th></tr>
<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Call Stack</th></tr>
<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Memory</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>
<tr><td bgcolor='#eeeeec' align='center'>1</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>{main}(  )</td><td title='%sbug01613-002.php' bgcolor='#eeeeec'>%sbug01613-002.php<b>:</b>0</td></tr>
</table></font>
