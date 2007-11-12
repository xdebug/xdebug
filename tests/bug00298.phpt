--TEST--
Test for bug #298: xdebug_var_dump & multiline strings,
--SKIPIF--
<?php if (!extension_loaded("xdebug")) print "skip"; ?>
<?php if(version_compare(zend_version(), "2.0.0-dev", '<')) echo "skip Zend Engine 2 needed\n"; ?>
--INI--
--FILE--
<?php
ini_set('html_errors', '1');
$sql = "select * \n table from \nwhere condition";
xdebug_var_dump($sql);
?>
--EXPECT--
<pre class='xdebug-var-dump' dir='ltr'><small>string</small> <font color='#cc0000'>'select * &#10; table from &#10;where condition'</font> <i>(length=38)</i>
</pre>
