--TEST--
Test for bug #1282: var_dump() of integers > 32 bit is broken on Windows
--SKIPIF--
<?php if (PHP_INT_SIZE != 8) { echo "skip Only for 64bit platforms"; } ?>
--INI--
xdebug.default_enable=1
xdebug.overload_var_dump=1
--FILE--
<?php
ini_set( 'html_errors', 0 );
var_dump(PHP_INT_MAX);

ini_set( 'html_errors', 1 );
var_dump(PHP_INT_MAX);
?>
--EXPECTF--
int(9223372036854775807)
<pre class='xdebug-var-dump' dir='ltr'><small>int</small> <font color='#4e9a06'>9223372036854775807</font>
</pre>
