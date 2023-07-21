--TEST--
Test for correct display with non overloaded var_dump()
--INI--
xdebug.mode=off
html_errors=1
date.timezone=Europe/Oslo
xdebug.var_display_max_children=11
xdebug.file_link_format=
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
xdebug_var_dump($ts1);
echo "\n";
var_dump($ts1);
?>
--EXPECTF--
object(TimeStuff)#%d (1) {
  ["timestamp%sprivat%s]=>
  int(1092515106)
}
<pre class='xdebug-var-dump' dir='ltr'>
<small>%sxdebug_var_dump_non_overload.php:14:</small>
<b>object</b>(<i>TimeStuff</i>)[<i>%d</i>]
  <i>private</i> 'timestamp' <font color='#888a85'>=&gt;</font> <small>int</small> <font color='#4e9a06'>1092515106</font>
</pre>
object(TimeStuff)#%d (1) {
  ["timestamp%sprivat%s]=>
  int(1092515106)
}
