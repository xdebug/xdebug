--TEST--
Test for stack traces with variadics (html, 3)
--SKIPIF--
<?php if (!version_compare(phpversion(), "5.6", '>=')) echo "skip >= PHP 5.6 needed\n"; ?>
--INI--
xdebug.default_enable=1
xdebug.profiler_enable=0
xdebug.auto_trace=0
xdebug.trace_format=0
xdebug.dump_globals=0
xdebug.show_mem_delta=0
xdebug.collect_vars=0
xdebug.collect_params=3
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.force_error_reporting=0
html_errors=1
--FILE--
<?php
function foo( $a, ...$b )
{
	trigger_error( 'notice' );
}

foo( 42 );
foo( 1, false );
foo( "foo", "bar", 3.1415 );
?>
--EXPECTF--
<br />
<font size='1'><table class='xdebug-error xe-notice' dir='ltr' border='1' cellspacing='0' cellpadding='1'>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> Notice: notice in %sstacktrace_variadic_html_3.php on line <i>4</i></th></tr>
<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Call Stack</th></tr>
<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Memory</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>
<tr><td bgcolor='#eeeeec' align='center'>1</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>{main}(  )</td><td title='%sstacktrace_variadic_html_3.php' bgcolor='#eeeeec'>...%sstacktrace_variadic_html_3.php<b>:</b>0</td></tr>
<tr><td bgcolor='#eeeeec' align='center'>2</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>foo( <span>42</span>, ...<i>variadic</i>() )</td><td title='%sstacktrace_variadic_html_3.php' bgcolor='#eeeeec'>...%sstacktrace_variadic_html_3.php<b>:</b>7</td></tr>
<tr><td bgcolor='#eeeeec' align='center'>3</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'><a href='http://www.php.net/function.trigger-error.html' target='_new'>trigger_error</a>
( <span>&#39;notice&#39;</span> )</td><td title='%sstacktrace_variadic_html_3.php' bgcolor='#eeeeec'>...%sstacktrace_variadic_html_3.php<b>:</b>4</td></tr>
</table></font>
<br />
<font size='1'><table class='xdebug-error xe-notice' dir='ltr' border='1' cellspacing='0' cellpadding='1'>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> Notice: notice in %sstacktrace_variadic_html_3.php on line <i>4</i></th></tr>
<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Call Stack</th></tr>
<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Memory</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>
<tr><td bgcolor='#eeeeec' align='center'>1</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>{main}(  )</td><td title='%sstacktrace_variadic_html_3.php' bgcolor='#eeeeec'>...%sstacktrace_variadic_html_3.php<b>:</b>0</td></tr>
<tr><td bgcolor='#eeeeec' align='center'>2</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>foo( <span>1</span>, ...<i>variadic</i>(<span>FALSE</span>) )</td><td title='%sstacktrace_variadic_html_3.php' bgcolor='#eeeeec'>...%sstacktrace_variadic_html_3.php<b>:</b>8</td></tr>
<tr><td bgcolor='#eeeeec' align='center'>3</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'><a href='http://www.php.net/function.trigger-error.html' target='_new'>trigger_error</a>
( <span>&#39;notice&#39;</span> )</td><td title='%sstacktrace_variadic_html_3.php' bgcolor='#eeeeec'>...%sstacktrace_variadic_html_3.php<b>:</b>4</td></tr>
</table></font>
<br />
<font size='1'><table class='xdebug-error xe-notice' dir='ltr' border='1' cellspacing='0' cellpadding='1'>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> Notice: notice in %sstacktrace_variadic_html_3.php on line <i>4</i></th></tr>
<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Call Stack</th></tr>
<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Memory</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>
<tr><td bgcolor='#eeeeec' align='center'>1</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>{main}(  )</td><td title='%sstacktrace_variadic_html_3.php' bgcolor='#eeeeec'>...%sstacktrace_variadic_html_3.php<b>:</b>0</td></tr>
<tr><td bgcolor='#eeeeec' align='center'>2</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>foo( <span>&#39;foo&#39;</span>, ...<i>variadic</i>(<span>&#39;bar&#39;</span>, <span>3.1415</span>) )</td><td title='%sstacktrace_variadic_html_3.php' bgcolor='#eeeeec'>...%sstacktrace_variadic_html_3.php<b>:</b>9</td></tr>
<tr><td bgcolor='#eeeeec' align='center'>3</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'><a href='http://www.php.net/function.trigger-error.html' target='_new'>trigger_error</a>
( <span>&#39;notice&#39;</span> )</td><td title='%sstacktrace_variadic_html_3.php' bgcolor='#eeeeec'>...%sstacktrace_variadic_html_3.php<b>:</b>4</td></tr>
</table></font>
