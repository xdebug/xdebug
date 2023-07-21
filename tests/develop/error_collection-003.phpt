--TEST--
Test for collection errors (3) - HTML errors
--INI--
display_errors=1
xdebug.mode=develop
html_errors=1
xdebug.file_link_format=xdebug://%f@%l
xdebug.filename_format=
xdebug.collect_assignments=1
xdebug.collect_return=1
xdebug.var_display_max_data=-1
--FILE--
<?php
xdebug_start_error_collection();

trigger_error("An error", E_USER_WARNING);

echo "Errors\n";
ini_set('html_errors', 0);
var_dump( xdebug_get_collected_errors() );
?>
--EXPECTF--
Errors
%serror_collection-003.php:8:
array(1) {
  [0] =>
  string(%d) "<br />
<font size='1'><table class='xdebug-error xe-warning' dir='ltr' border='1' cellspacing='0' cellpadding='1'>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> Warning: An error in <a style='color: black' href='xdebug://%serror_collection-003.php@4'>%serror_collection-003.php</a> on line <i>4</i></th></tr>
<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Call Stack</th></tr>
<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Memory</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>
<tr><td bgcolor='#eeeeec' align='center'>1</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>{main}(  )</td><td title='%serror_collection-003.php' bgcolor='#eeeeec'><a style='color: black' href='xdebug://%serror_collection-003.php@0'>...%serror_collection-003.php<b>:</b>0</a></td></tr>
<tr><td bgcolor='#eeeeec' align='center'>2</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'><a href='http://www.php.net/function.trigger-error.html' target='_new'>trigger_error</a>( <span>$message = </span><span>&#39;An error&#39;</span>, <span>$error_%s = </span><span>512</span> )</td><td title='%serror_collection-003.php' bgcolor='#eeeeec'><a style='color: black' href='xdebug://%serror_collection-003.php@4'>...%serror_collection-003.php<b>:</b>4</a></td></tr>
</table></font>
"
}
