--TEST--
Test for bug #1837: Support for associative variadic variable names in HTML stack traces
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.0');
?>
--INI--
html_errors=1
xdebug.mode=develop
--FILE--
<?php
function takeThemAll(string $one, ...$args)
{
	xdebug_print_function_stack();
}

takeThemAll(one: "test", arg1: 42, arg2: M_PI);
takeThemAll(arg1: 42, one: "test", arg2: M_PI);
takeThemAll("test", arg1: 42, arg2: M_PI);
takeThemAll("test", 42, M_PI);
takeThemAll("test", 42, arg2: M_PI);
?>
--EXPECTF--
<br />
<font size='1'><table class='xdebug-error xe-xdebug' dir='ltr' border='1' cellspacing='0' cellpadding='1'>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> Xdebug: user triggered in %sbug01837-003.php on line <i>4</i></th></tr>
<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Call Stack</th></tr>
<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Memory</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>
<tr><td bgcolor='#eeeeec' align='center'>1</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>{main}(  )</td><td title='%sbug01837-003.php' bgcolor='#eeeeec'>%sbug01837-003.php<b>:</b>0</td></tr>
<tr><td bgcolor='#eeeeec' align='center'>2</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>takeThemAll( <span>$one = </span><span>&#39;test&#39;</span>, ...<span>$args = </span><i>variadic</i>(<span>$arg1 = </span><span>42</span>, <span>$arg2 = </span><span>3.1415926535898</span>) )</td><td title='%sbug01837-003.php' bgcolor='#eeeeec'>%sbug01837-003.php<b>:</b>7</td></tr>
<tr><td bgcolor='#eeeeec' align='center'>3</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'><a href='http://www.php.net/function.xdebug-print-function-stack.html' target='_new'>xdebug_print_function_stack</a>(  )</td><td title='%sbug01837-003.php' bgcolor='#eeeeec'>%sbug01837-003.php<b>:</b>4</td></tr>
</table></font>
<br />
<font size='1'><table class='xdebug-error xe-xdebug' dir='ltr' border='1' cellspacing='0' cellpadding='1'>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> Xdebug: user triggered in %sbug01837-003.php on line <i>4</i></th></tr>
<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Call Stack</th></tr>
<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Memory</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>
<tr><td bgcolor='#eeeeec' align='center'>1</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>{main}(  )</td><td title='%sbug01837-003.php' bgcolor='#eeeeec'>%sbug01837-003.php<b>:</b>0</td></tr>
<tr><td bgcolor='#eeeeec' align='center'>2</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>takeThemAll( <span>$one = </span><span>&#39;test&#39;</span>, ...<span>$args = </span><i>variadic</i>(<span>$arg1 = </span><span>42</span>, <span>$arg2 = </span><span>3.1415926535898</span>) )</td><td title='%sbug01837-003.php' bgcolor='#eeeeec'>%sbug01837-003.php<b>:</b>8</td></tr>
<tr><td bgcolor='#eeeeec' align='center'>3</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'><a href='http://www.php.net/function.xdebug-print-function-stack.html' target='_new'>xdebug_print_function_stack</a>(  )</td><td title='%sbug01837-003.php' bgcolor='#eeeeec'>%sbug01837-003.php<b>:</b>4</td></tr>
</table></font>
<br />
<font size='1'><table class='xdebug-error xe-xdebug' dir='ltr' border='1' cellspacing='0' cellpadding='1'>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> Xdebug: user triggered in %sbug01837-003.php on line <i>4</i></th></tr>
<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Call Stack</th></tr>
<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Memory</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>
<tr><td bgcolor='#eeeeec' align='center'>1</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>{main}(  )</td><td title='%sbug01837-003.php' bgcolor='#eeeeec'>%sbug01837-003.php<b>:</b>0</td></tr>
<tr><td bgcolor='#eeeeec' align='center'>2</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>takeThemAll( <span>$one = </span><span>&#39;test&#39;</span>, ...<span>$args = </span><i>variadic</i>(<span>$arg1 = </span><span>42</span>, <span>$arg2 = </span><span>3.1415926535898</span>) )</td><td title='%sbug01837-003.php' bgcolor='#eeeeec'>%sbug01837-003.php<b>:</b>9</td></tr>
<tr><td bgcolor='#eeeeec' align='center'>3</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'><a href='http://www.php.net/function.xdebug-print-function-stack.html' target='_new'>xdebug_print_function_stack</a>(  )</td><td title='%sbug01837-003.php' bgcolor='#eeeeec'>%sbug01837-003.php<b>:</b>4</td></tr>
</table></font>
<br />
<font size='1'><table class='xdebug-error xe-xdebug' dir='ltr' border='1' cellspacing='0' cellpadding='1'>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> Xdebug: user triggered in %sbug01837-003.php on line <i>4</i></th></tr>
<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Call Stack</th></tr>
<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Memory</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>
<tr><td bgcolor='#eeeeec' align='center'>1</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>{main}(  )</td><td title='%sbug01837-003.php' bgcolor='#eeeeec'>%sbug01837-003.php<b>:</b>0</td></tr>
<tr><td bgcolor='#eeeeec' align='center'>2</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>takeThemAll( <span>$one = </span><span>&#39;test&#39;</span>, ...<span>$args = </span><i>variadic</i>(<span>42</span>, <span>3.1415926535898</span>) )</td><td title='%sbug01837-003.php' bgcolor='#eeeeec'>%sbug01837-003.php<b>:</b>10</td></tr>
<tr><td bgcolor='#eeeeec' align='center'>3</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'><a href='http://www.php.net/function.xdebug-print-function-stack.html' target='_new'>xdebug_print_function_stack</a>(  )</td><td title='%sbug01837-003.php' bgcolor='#eeeeec'>%sbug01837-003.php<b>:</b>4</td></tr>
</table></font>
<br />
<font size='1'><table class='xdebug-error xe-xdebug' dir='ltr' border='1' cellspacing='0' cellpadding='1'>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> Xdebug: user triggered in %sbug01837-003.php on line <i>4</i></th></tr>
<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Call Stack</th></tr>
<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Memory</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>
<tr><td bgcolor='#eeeeec' align='center'>1</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>{main}(  )</td><td title='%sbug01837-003.php' bgcolor='#eeeeec'>%sbug01837-003.php<b>:</b>0</td></tr>
<tr><td bgcolor='#eeeeec' align='center'>2</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>takeThemAll( <span>$one = </span><span>&#39;test&#39;</span>, ...<span>$args = </span><i>variadic</i>(<span>42</span>, <span>$arg2 = </span><span>3.1415926535898</span>) )</td><td title='%sbug01837-003.php' bgcolor='#eeeeec'>%sbug01837-003.php<b>:</b>11</td></tr>
<tr><td bgcolor='#eeeeec' align='center'>3</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'><a href='http://www.php.net/function.xdebug-print-function-stack.html' target='_new'>xdebug_print_function_stack</a>(  )</td><td title='%sbug01837-003.php' bgcolor='#eeeeec'>%sbug01837-003.php<b>:</b>4</td></tr>
</table></font>
