--TEST--
Test for display limitations with xdebug_var_dump().
--INI--
xdebug.var_display_max_children=-1
xdebug.var_display_max_data=-1
xdebug.var_display_max_depth=-1
html_errors=1
xdebug.default_enable=1
xdebug.overload_var_dump=1
--FILE--
<?php
$array = array( 1, true, "string" );
xdebug_var_dump( $array ); echo "\n\n";

ini_set('xdebug.var_display_max_depth', 0);
xdebug_var_dump( $array ); echo "\n\n";

ini_set('xdebug.var_display_max_depth', -1);
ini_set('xdebug.var_display_max_data', 0);
xdebug_var_dump( $array ); echo "\n\n";

ini_set('xdebug.var_display_max_children', 0);
ini_set('xdebug.var_display_max_data', -1);
xdebug_var_dump( $array ); echo "\n\n";
?>
--EXPECT--
<pre class='xdebug-var-dump' dir='ltr'>
<b>array</b> <i>(size=3)</i>
  0 <font color='#888a85'>=&gt;</font> <small>int</small> <font color='#4e9a06'>1</font>
  1 <font color='#888a85'>=&gt;</font> <small>boolean</small> <font color='#75507b'>true</font>
  2 <font color='#888a85'>=&gt;</font> <small>string</small> <font color='#cc0000'>'string'</font> <i>(length=6)</i>
</pre>

<pre class='xdebug-var-dump' dir='ltr'>
<b>array</b> <i>(size=3)</i>
  ...
</pre>

<pre class='xdebug-var-dump' dir='ltr'>
<b>array</b> <i>(size=3)</i>
  0 <font color='#888a85'>=&gt;</font> <small>int</small> <font color='#4e9a06'>1</font>
  1 <font color='#888a85'>=&gt;</font> <small>boolean</small> <font color='#75507b'>true</font>
  2 <font color='#888a85'>=&gt;</font> <small>string</small> <font color='#cc0000'>''...</font> <i>(length=6)</i>
</pre>

<pre class='xdebug-var-dump' dir='ltr'>
<b>array</b> <i>(size=3)</i>
  <i>more elements...</i>
</pre>
