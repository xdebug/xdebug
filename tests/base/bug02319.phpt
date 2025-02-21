--TEST--
Test for bug #2319: Emojis are not handled correctly in xdebug's var_dumps
--INI--
xdebug.mode=develop
html_errors=1
--FILE--
<?php
$str = 'hello 👍';
var_dump($str);
?>
--EXPECTF--
<pre class='xdebug-var-dump' dir='ltr'>
<small>%sbug02319.php:3:</small><small>string</small> <font color='#cc0000'>'hello 👍'</font> <i>(length=10)</i>
</pre>
