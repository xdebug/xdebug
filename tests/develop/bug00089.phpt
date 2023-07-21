--TEST--
Test for bug #89: var_dump shows empty strings garbled
--INI--
xdebug.mode=develop
xdebug.show_local_vars=0
html_errors=1
xdebug.var_display_max_children=3
xdebug.file_link_format=
xdebug.filename_format=
--FILE--
<?php
var_dump(array(4, array('', 2, 'node'), false));
?>
--EXPECTF--
<pre class='xdebug-var-dump' dir='ltr'>
<small>%sbug00089.php:2:</small>
<b>array</b> <i>(size=3)</i>
  0 <font color='#888a85'>=&gt;</font> <small>int</small> <font color='#4e9a06'>4</font>
  1 <font color='#888a85'>=&gt;</font> 
    <b>array</b> <i>(size=3)</i>
      0 <font color='#888a85'>=&gt;</font> <small>string</small> <font color='#cc0000'>''</font> <i>(length=0)</i>
      1 <font color='#888a85'>=&gt;</font> <small>int</small> <font color='#4e9a06'>2</font>
      2 <font color='#888a85'>=&gt;</font> <small>string</small> <font color='#cc0000'>'node'</font> <i>(length=4)</i>
  2 <font color='#888a85'>=&gt;</font> <small>boolean</small> <font color='#75507b'>false</font>
</pre>
