--TEST--
Test for bug #903: xdebug_get_headers() returns replaced headers
--INI--
xdebug.default_enable=1
xdebug.overload_var_dump=1
--FILE--
<?php
header('foo: bar');
header('foo: baz');

setcookie('test1', 'one');
setcookie('test2', 'two');

header('whoop: baz');

header('Set-Cookie: remove');

var_dump(xdebug_get_headers());
?>
--EXPECTF--
array(3) {
  [0] =>
  string(8) "foo: baz"
  [1] =>
  string(10) "whoop: baz"
  [2] =>
  string(18) "Set-Cookie: remove"
}
