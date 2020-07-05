--TEST--
Test for bug #800: var_dump(get_class(new foo\bar')) add an extra "\" in class name.
--INI--
xdebug.mode=develop
--FILE--
<?php

namespace foo;

class bar {}

var_dump(get_class(new bar));
?>
--EXPECTF--
%sbug00800.php:7:
string(7) "foo\bar"
