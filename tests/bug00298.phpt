--TEST--
Test for bug #298: xdebug_var_dump & multiline strings
--INI--
xdebug.default_enable=1
--FILE--
<?php
ini_set('html_errors', '1');
$sql = "select * \n table from \nwhere condition";
xdebug_var_dump($sql);
?>
--EXPECT--
<pre class='xdebug-var-dump' dir='ltr'><small>string</small> <font color='#cc0000'>'select * &#10; table from &#10;where condition'</font> <i>(length=38)</i>
</pre>
