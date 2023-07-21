--TEST--
Test for bug #280: var_dump don't display key of array as expected
--INI--
xdebug.mode=develop
xdebug.file_link_format=
--FILE--
<?php
$var = "te\0st";
$arr = array($var=>$var);
ini_set('html_errors', '1');
var_dump($arr);
?>
--EXPECTF--
<pre class='xdebug-var-dump' dir='ltr'>
<small>%sbug00280.php:5:</small>
<b>array</b> <i>(size=1)</i>
  'te&#0;st' <font color='#888a85'>=&gt;</font> <small>string</small> <font color='#cc0000'>'te&#0;st'</font> <i>(length=5)</i>
</pre>
