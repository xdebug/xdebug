--TEST--
Test for file display with xdebug_var_dump()
--INI--
xdebug.mode=develop
html_errors=1
xdebug.var_display_max_children=11
xdebug.file_link_format=
xdebug.filename_format=
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

ini_set('xdebug.filename_format', "%n");
var_dump($ts1);
echo "\n";

ini_set('xdebug.filename_format', "…%s%p");
var_dump($ts1);
echo "\n";

ini_set('xdebug.filename_format', "«%a»");
var_dump($ts1);
echo "\n";

ini_set('xdebug.filename_format', "{%f}");
var_dump($ts1);
echo "\n";
?>
--EXPECTF--
<pre class='xdebug-var-dump' dir='ltr'>
<small>xdebug_var_dump_fileformat-html.php:%d:</small>
<b>object</b>(<i>TimeStuff</i>)[<i>%d</i>]
  <i>private</i> 'timestamp' <font color='#888a85'>=&gt;</font> <small>int</small> <font color='#4e9a06'>1092515106</font>
</pre>
<pre class='xdebug-var-dump' dir='ltr'>
<small>…%edevelop%exdebug_var_dump_fileformat-html.php:%d:</small>
<b>object</b>(<i>TimeStuff</i>)[<i>%d</i>]
  <i>private</i> 'timestamp' <font color='#888a85'>=&gt;</font> <small>int</small> <font color='#4e9a06'>1092515106</font>
</pre>
<pre class='xdebug-var-dump' dir='ltr'>
<small>«tests%edevelop%exdebug_var_dump_fileformat-html.php»:%d:</small>
<b>object</b>(<i>TimeStuff</i>)[<i>%d</i>]
  <i>private</i> 'timestamp' <font color='#888a85'>=&gt;</font> <small>int</small> <font color='#4e9a06'>1092515106</font>
</pre>
<pre class='xdebug-var-dump' dir='ltr'>
<small>{%stests%edevelop%exdebug_var_dump_fileformat-html.php}:%d:</small>
<b>object</b>(<i>TimeStuff</i>)[<i>%d</i>]
  <i>private</i> 'timestamp' <font color='#888a85'>=&gt;</font> <small>int</small> <font color='#4e9a06'>1092515106</font>
</pre>
