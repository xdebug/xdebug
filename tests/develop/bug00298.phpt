--TEST--
Test for bug #298: xdebug_var_dump & multiline strings
--INI--
xdebug.mode=develop
xdebug.file_link_format=
xdebug.filename_format=
--FILE--
<?php
ini_set('html_errors', '1');
$sql = "select * \n table from \nwhere condition";
xdebug_var_dump($sql);
?>
--EXPECTF--
<pre class='xdebug-var-dump' dir='ltr'>
<small>%sbug00298.php:4:</small><small>string</small> <font color='#cc0000'>'select * &#10; table from &#10;where condition'</font> <i>(length=38)</i>
</pre>
