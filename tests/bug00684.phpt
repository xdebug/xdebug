--TEST--
Test for bug #684: xdebug_var_dump - IE does not support &amp;
--INI--
html_errors=1
xdebug.default_enable=1
--FILE--
<?php
xdebug_var_dump('Testing isn\'t fun');
?>
--EXPECT--
<pre class='xdebug-var-dump' dir='ltr'><small>string</small> <font color='#cc0000'>'Testing isn&#39;t fun'</font> <i>(length=17)</i>
</pre>
