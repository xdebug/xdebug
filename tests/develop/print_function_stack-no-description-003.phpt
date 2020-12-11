--TEST--
Test for xdebug_print_function_stack() without description (HTML)
--INI--
xdebug.mode=develop
xdebug.cli_color=0
xdebug.file_link_format=
xdebug.filename_format=
html_errors=1
--FILE--
<?php
function foo()
{
	xdebug_print_function_stack("test message", XDEBUG_STACK_NO_DESC);
}

function bar()
{
	foo(1, 3, 'foo', 'bar' );
}

bar();
?>
--EXPECTF--
<br />
<font size='1'><table class='xdebug-error xe-xdebug' dir='ltr' border='1' cellspacing='0' cellpadding='1'>
<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Call Stack</th></tr>
<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Memory</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>
<tr><td bgcolor='#eeeeec' align='center'>1</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>{main}(  )</td><td title='%sprint_function_stack-no-description-003.php' bgcolor='#eeeeec'>...%sprint_function_stack-no-description-003.php<b>:</b>0</td></tr>
<tr><td bgcolor='#eeeeec' align='center'>2</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>bar(  )</td><td title='%sprint_function_stack-no-description-003.php' bgcolor='#eeeeec'>...%sprint_function_stack-no-description-003.php<b>:</b>12</td></tr>
<tr><td bgcolor='#eeeeec' align='center'>3</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>foo( <span>1</span>, <span>3</span>, <span>&#39;foo&#39;</span>, <span>&#39;bar&#39;</span> )</td><td title='%sprint_function_stack-no-description-003.php' bgcolor='#eeeeec'>...%sprint_function_stack-no-description-003.php<b>:</b>9</td></tr>
<tr><td bgcolor='#eeeeec' align='center'>4</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'><a href='http://www.php.net/function.xdebug-print-function-stack.html' target='_new'>xdebug_print_function_stack</a>( <span>$message = </span><span>&#39;test message&#39;</span>, <span>$options = </span><span>1</span> )</td><td title='%sprint_function_stack-no-description-003.php' bgcolor='#eeeeec'>...%sprint_function_stack-no-description-003.php<b>:</b>4</td></tr>
</table></font>
