--TEST--
Test for bug #280: var_dump don't display key of array as expected
--SKIPIF--
<?php if (!extension_loaded("xdebug")) print "skip"; ?>
<?php if(version_compare(zend_version(), "2.0.0-dev", '<')) echo "skip Zend Engine 2 needed\n"; ?>
--INI--
--FILE--
<?php
$var = "te\0st";
$arr = array($var=>$var);
ini_set('html_errors', '1');
var_dump($arr);
?>
--EXPECT--
<pre class='xdebug-var-dump' dir='ltr'>
<b>array</b>
  'te&#0;st' <font color='#888a85'>=&gt;</font> <small>string</small> <font color='#cc0000'>'te&#0;st'</font> <i>(length=5)</i>
</pre>
