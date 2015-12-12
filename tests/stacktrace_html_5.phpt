--TEST--
Test stack traces (level5, html)
--INI--
xdebug.default_enable=1
xdebug.profiler_enable=0
xdebug.auto_trace=0
xdebug.trace_format=0
xdebug.dump_globals=0
xdebug.show_mem_delta=0
xdebug.var_display_max_children=50
xdebug.var_display_max_depth=5
xdebug.var_display_max_length=64
xdebug.collect_vars=1
xdebug.collect_params=5
xdebug.collect_return=0
html_errors=1
xdebug.file_link_format=
xdebug.show_local_vars=0
xdebug.show_error_trace=1
--FILE--
<?php
function foo( $a ) {
    for ($i = 1; $i < $a['foo']; $i++) {
		poo();
    }
}

$c = new stdClass;
$c->bar = 100;
$a = array(
    42 => false, 'foo' => 912124,
    $c, new stdClass, fopen( __FILE__, 'r' )
);
try { foo( $a ); } catch (Throwable $e) { /* ignore */ }
?>
--EXPECTF--
<br />
<font size='1'><table class='xdebug-error xe-%s' dir='ltr' border='1' cellspacing='0' cellpadding='1'>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> %srror: Call to undefined function poo() in %sstacktrace_html_5.php on line <i>4</i></th></tr>
<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Call Stack</th></tr>
<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Memory</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>
<tr><td bgcolor='#eeeeec' align='center'>1</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>{main}(  )</td><td title='%sstacktrace_html_5.php' bgcolor='#eeeeec'>...%sstacktrace_html_5.php<b>:</b>0</td></tr>
<tr><td bgcolor='#eeeeec' align='center'>2</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>foo( <span>YTo1OntpOjQyO2I6MDtzOjM6ImZvbyI7aTo5MTIxMjQ7aTo0MztPOjg6InN0ZENsYXNzIjoxOntzOjM6ImJhciI7aToxMDA7fWk6NDQ7Tzo4OiJzdGRDbGFzcyI6MDp7fWk6NDU7aTowO30=</span> )</td><td title='%sstacktrace_html_5.php' bgcolor='#eeeeec'>...%sstacktrace_html_5.php<b>:</b>14</td></tr>
</table></font>
