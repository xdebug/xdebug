--TEST--
Test for bug #280: var_dump don't display key of array as expected
--INI--
xdebug.overload_var_dump=1
xdebug.default_enable=1
--FILE--
<?php
$var = "te\0st";
$arr = array($var=>$var);
ini_set('html_errors', '1');
var_dump($arr);
?>
--EXPECT--
<pre class='xdebug-var-dump' dir='ltr'>
<b>array</b> <i>(size=1)</i>
  'te&#0;st' <font color='#888a85'>=&gt;</font> <small>string</small> <font color='#cc0000'>'te&#0;st'</font> <i>(length=5)</i>
</pre>
