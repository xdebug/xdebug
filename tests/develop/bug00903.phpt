--TEST--
Test for bug #903: xdebug_get_headers() returns replaced headers
--INI--
xdebug.mode=develop
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
%sbug00903.php:12:
array(3) {
  [0] =>
  string(8) "foo: baz"
  [1] =>
  string(10) "whoop: baz"
  [2] =>
  string(18) "Set-Cookie: remove"
}
