--TEST--
Test for correct display with non overloaded var_dump()
--INI--
xdebug.default_enable=1
xdebug.auto_trace=0
xdebug.profiler_enable=0
html_errors=1
date.timezone=Europe/Oslo
xdebug.var_display_max_children=11
xdebug.overload_var_dump=0
--FILE--
<?php
	class TimeStuff {
		private $timestamp;

		function __construct($ts = null)
		{
			$this->timestamp = $ts === null ? time() : $ts;
		}
	}

	$ts1 = new TimeStuff(1092515106);

	var_dump($ts1);
	ini_set('xdebug.overload_var_dump', 1);
	var_dump($ts1);
	echo "\n";
	ini_set('xdebug.overload_var_dump', 0);
	var_dump($ts1);
?>
--EXPECTF--
object(TimeStuff)#1 (1) {
  ["timestamp%sprivat%s]=>
  int(1092515106)
}
<pre class='xdebug-var-dump' dir='ltr'>
<b>object</b>(<i>TimeStuff</i>)[<i>1</i>]
  <i>private</i> 'timestamp' <font color='#888a85'>=&gt;</font> <small>int</small> <font color='#4e9a06'>1092515106</font>
</pre>
object(TimeStuff)#1 (1) {
  ["timestamp%sprivat%s]=>
  int(1092515106)
}
