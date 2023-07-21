--TEST--
Test for bug #1262: overload_var_dump=0 messes with xdebug_var_dump()
--INI--
xdebug.mode=off
html_errors=1
xdebug.file_link_format=
--FILE--
<?php
xdebug_var_dump("Anything");
echo "\n";
var_dump("Anything");
?>
--EXPECTF--
<pre class='xdebug-var-dump' dir='ltr'>
<small>%sbug01262.php:2:</small><small>string</small> <font color='#cc0000'>'Anything'</font> <i>(length=8)</i>
</pre>
string(8) "Anything"
