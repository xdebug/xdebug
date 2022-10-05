--TEST--
Test for bug #2121: Xdebug does not use local independent float-to-string functions
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('!win');
if (false == setlocale(LC_ALL, "da_DK.utf8")) print "skip locale not found";
?>
--INI--
xdebug.mode=develop
--FILE--
<?php
setlocale(LC_ALL, "da_DK.utf8");

ini_set('html_errors', 0);

ini_set('xdebug.cli_color', 0);
var_dump(54.234);

ini_set('xdebug.cli_color', 2);
var_dump(54.234);

ini_set('html_errors', 1);
var_dump(54.234);
?>
--EXPECTF--
%s:%d:
double(54.234)
[1m%s[22m:[1m%d[22m:
[1mdouble[22m([33m54.234[0m)
<pre class='xdebug-var-dump' dir='ltr'>
<small>%s:%d:</small><small>float</small> <font color='#f57900'>54.234</font>
</pre>
