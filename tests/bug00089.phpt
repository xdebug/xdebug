--TEST--
Test for bug #89: var_dump shows empty strings garbled
--INI--
xdebug.default_enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
xdebug.profiler_enable=0
xdebug.show_local_vars=0
html_errors=1
--FILE--
<?php
var_dump(array(4, array('', 2, 'node'), false));
?>
--EXPECT--
<pre>
<b>array</b>
  0 <font color='#777777'>=&gt;</font> <font color='#00bb00'>4</font>
  1 <font color='#777777'>=&gt;</font> 
    <b>array</b>
      0 <font color='#777777'>=&gt;</font> <font color='#bb00bb'>''</font>
      1 <font color='#777777'>=&gt;</font> <font color='#00bb00'>2</font>
      2 <font color='#777777'>=&gt;</font> <font color='#bb00bb'>'node'</font>
  2 <font color='#777777'>=&gt;</font> <font color='#0000ff'>false</font>
</pre>
