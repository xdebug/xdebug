--TEST--
Trace test with fibonacci numbers (format=2)
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.trace_format=2
--FILE--
<?php
require_once 'capture-trace.inc';
function fibonacci_cache ($n)
{
	if (isset ($GLOBALS['fcache'][$n])) {
		return $GLOBALS['fcache'][$n];
	}

	if ($n == 0) {
		return 0;
	} else if ($n == 1) {
		return 1;
	} else if ($n == 2) {
		return 1;
	} else {
		$t = fibonacci_cache($n - 1) + fibonacci_cache($n - 2);
		$GLOBALS['fcache'][$n] = $t;
		return $t;
	}
}

fibonacci_cache(10);
xdebug_stop_trace();
?>
--EXPECTF--
<table style='hyphens: auto; -webkit-hyphens: auto; -ms-hyphens: auto;' class='xdebug-trace' dir='ltr' border='1' cellspacing='0'>
	<tr><th>#</th><th>Time</th><th>Mem</th><th colspan='2'>Function</th><th>Location</th></tr>
	<tr><td>%d</td><td>%f</td><td align='right'>%d</td><td align='left'>&nbsp; &nbsp;-&gt;</td><td>fibonacci_cache()</td><td>%strace-002.php:22</td></tr>
	<tr><td>%d</td><td>%f</td><td align='right'>%d</td><td align='left'>&nbsp; &nbsp;&nbsp; &nbsp;-&gt;</td><td>fibonacci_cache()</td><td>%strace-002.php:16</td></tr>
	<tr><td>%d</td><td>%f</td><td align='right'>%d</td><td align='left'>&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;-&gt;</td><td>fibonacci_cache()</td><td>%strace-002.php:16</td></tr>
	<tr><td>%d</td><td>%f</td><td align='right'>%d</td><td align='left'>&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;-&gt;</td><td>fibonacci_cache()</td><td>%strace-002.php:16</td></tr>
	<tr><td>%d</td><td>%f</td><td align='right'>%d</td><td align='left'>&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;-&gt;</td><td>fibonacci_cache()</td><td>%strace-002.php:16</td></tr>
	<tr><td>%d</td><td>%f</td><td align='right'>%d</td><td align='left'>&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;-&gt;</td><td>fibonacci_cache()</td><td>%strace-002.php:16</td></tr>
	<tr><td>%d</td><td>%f</td><td align='right'>%d</td><td align='left'>&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;-&gt;</td><td>fibonacci_cache()</td><td>%strace-002.php:16</td></tr>
	<tr><td>%d</td><td>%f</td><td align='right'>%d</td><td align='left'>&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;-&gt;</td><td>fibonacci_cache()</td><td>%strace-002.php:16</td></tr>
	<tr><td>%d</td><td>%f</td><td align='right'>%d</td><td align='left'>&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;-&gt;</td><td>fibonacci_cache()</td><td>%strace-002.php:16</td></tr>
	<tr><td>%d</td><td>%f</td><td align='right'>%d</td><td align='left'>&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;-&gt;</td><td>fibonacci_cache()</td><td>%strace-002.php:16</td></tr>
	<tr><td>%d</td><td>%f</td><td align='right'>%d</td><td align='left'>&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;-&gt;</td><td>fibonacci_cache()</td><td>%strace-002.php:16</td></tr>
	<tr><td>%d</td><td>%f</td><td align='right'>%d</td><td align='left'>&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;-&gt;</td><td>fibonacci_cache()</td><td>%strace-002.php:16</td></tr>
	<tr><td>%d</td><td>%f</td><td align='right'>%d</td><td align='left'>&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;-&gt;</td><td>fibonacci_cache()</td><td>%strace-002.php:16</td></tr>
	<tr><td>%d</td><td>%f</td><td align='right'>%d</td><td align='left'>&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;-&gt;</td><td>fibonacci_cache()</td><td>%strace-002.php:16</td></tr>
	<tr><td>%d</td><td>%f</td><td align='right'>%d</td><td align='left'>&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;-&gt;</td><td>fibonacci_cache()</td><td>%strace-002.php:16</td></tr>
	<tr><td>%d</td><td>%f</td><td align='right'>%d</td><td align='left'>&nbsp; &nbsp;&nbsp; &nbsp;&nbsp; &nbsp;-&gt;</td><td>fibonacci_cache()</td><td>%strace-002.php:16</td></tr>
	<tr><td>%d</td><td>%f</td><td align='right'>%d</td><td align='left'>&nbsp; &nbsp;&nbsp; &nbsp;-&gt;</td><td>fibonacci_cache()</td><td>%strace-002.php:16</td></tr>
	<tr><td>%d</td><td>%f</td><td align='right'>%d</td><td align='left'>&nbsp; &nbsp;-&gt;</td><td>xdebug_stop_trace()</td><td>%strace-002.php:23</td></tr>
</table>
