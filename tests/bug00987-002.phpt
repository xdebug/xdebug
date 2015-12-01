--TEST--
Test for bug #987: Hidden property names not shown with var_dump (HTML)
--INI--
html_errors=1
xdebug.cli_color=0
xdebug.default_enable=1
xdebug.overload_var_dump=2
xdebug.file_link_format=
--FILE--
<?php
$object = (object) array('key' => 'value', 1 => 0, -4 => "foo", 3.14 => false);

var_dump($object);
?>
--EXPECTF--
<pre class='xdebug-var-dump' dir='ltr'>
<small>%sbug00987-002.php:4:</small>
<b>object</b>(<i>stdClass</i>)[<i>1</i>]
  <i>public</i> 'key' <font color='#888a85'>=&gt;</font> <small>string</small> <font color='#cc0000'>'value'</font> <i>(length=5)</i>
  <i>public</i> 1 <font color='#888a85'>=&gt;</font> <small>int</small> <font color='#4e9a06'>0</font>
  <i>public</i> -4 <font color='#888a85'>=&gt;</font> <small>string</small> <font color='#cc0000'>'foo'</font> <i>(length=3)</i>
  <i>public</i> 3 <font color='#888a85'>=&gt;</font> <small>boolean</small> <font color='#75507b'>false</font>
</pre>
