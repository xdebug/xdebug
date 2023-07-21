--TEST--
Test for bug #1282: var_dump() of integers > 32 bit is broken on Windows
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('64bit');
?>
--INI--
xdebug.mode=develop
xdebug.file_link_format=
--FILE--
<?php
ini_set( 'html_errors', 0 );
var_dump(PHP_INT_MAX);

ini_set( 'html_errors', 1 );
var_dump(PHP_INT_MAX);
?>
--EXPECTF--
%sbug01282-64bit.php:3:
int(9223372036854775807)
<pre class='xdebug-var-dump' dir='ltr'>
<small>%sbug01282-64bit.php:6:</small><small>int</small> <font color='#4e9a06'>9223372036854775807</font>
</pre>
