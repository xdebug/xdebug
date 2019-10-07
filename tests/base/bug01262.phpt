--TEST--
Test for bug #1262: overload_var_dump=0 messes with xdebug_var_dump()
--INI--
xdebug.default_enable = 1
xdebug.overload_var_dump = 0
html_errors = 1
--FILE--
<?php
xdebug_var_dump("Anything");
echo "\n";
var_dump("Anything");
?>
--EXPECT--
<pre class='xdebug-var-dump' dir='ltr'><small>string</small> <font color='#cc0000'>'Anything'</font> <i>(length=8)</i>
</pre>
string(8) "Anything"
