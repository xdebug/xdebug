--TEST--
Test for bug #811: PHP Documentation Link (PHP < 8)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP < 8');
?>
--INI--
docref_root=http://www.php.net/
docref_ext=.php
html_errors=1
xdebug.mode=develop
xdebug.file_link_format=
xdebug.filename_format=
xdebug.show_error_trace=0
--FILE--
<?php
// normal function
date_create("hocuspocus", "fasdfasdf", "afsdfasdf");

// static method
DateTime::createFromFormat("fasdfdsaf", "fasdf", NULL, "fasdf");

// ctor
new ArrayObject("fasdfdsaf", "fasdf", "fasdf");
?>
--EXPECTF--
<br />
<font size='1'><table class='xdebug-error xe-warning' dir='ltr' border='1' cellspacing='0' cellpadding='1'>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> Warning: date_create() expects at most 2 parameters, 3 given in %sbug00811.php on line <i>3</i></th></tr>
<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Call Stack</th></tr>
<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Memory</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>
<tr><td bgcolor='#eeeeec' align='center'>1</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>{main}(  )</td><td title='%sbug00811.php' bgcolor='#eeeeec'>...%sbug00811.php<b>:</b>0</td></tr>
<tr><td bgcolor='#eeeeec' align='center'>2</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'><a href='http://www.php.net/function.date-create.php' target='_new'>date_create</a>(%s)</td><td title='%sbug00811.php' bgcolor='#eeeeec'>...%sbug00811.php<b>:</b>3</td></tr>
</table></font>
<br />
<font size='1'><table class='xdebug-error xe-warning' dir='ltr' border='1' cellspacing='0' cellpadding='1'>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> Warning: DateTime::createFromFormat() expects at most 3 parameters, 4 given in %sbug00811.php on line <i>6</i></th></tr>
<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Call Stack</th></tr>
<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Memory</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>
<tr><td bgcolor='#eeeeec' align='center'>1</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>{main}(  )</td><td title='%sbug00811.php' bgcolor='#eeeeec'>...%sbug00811.php<b>:</b>0</td></tr>
<tr><td bgcolor='#eeeeec' align='center'>2</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'><a href='http://www.php.net/DateTime.createFromFormat.php' target='_new'>createFromFormat</a>(%s)</td><td title='%sbug00811.php' bgcolor='#eeeeec'>...%sbug00811.php<b>:</b>6</td></tr>
</table></font>
<br />
<font size='1'><table class='xdebug-error xe-uncaught-exception' dir='ltr' border='1' cellspacing='0' cellpadding='1'>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> Fatal error: Uncaught%sin %sbug00811.php on line <i>9</i></th></tr>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> %s: ArrayObject::__construct() expects parameter 2 to be %r(long|integer|int)%r, string given in %sbug00811.php on line <i>9</i></th></tr>
<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Call Stack</th></tr>
<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Memory</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>
<tr><td bgcolor='#eeeeec' align='center'>1</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>{main}(  )</td><td title='%sbug00811.php' bgcolor='#eeeeec'>...%sbug00811.php<b>:</b>0</td></tr>
<tr><td bgcolor='#eeeeec' align='center'>2</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'><a href='http://www.php.net/ArrayObject.construct.php' target='_new'>__construct</a>(%s)</td><td title='%sbug00811.php' bgcolor='#eeeeec'>...%sbug00811.php<b>:</b>9</td></tr>
</table></font>
