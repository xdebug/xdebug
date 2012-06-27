--TEST--
Test for bug #800: var_dump(get_class(new foo\bar')) add an extra "\" in class name.
--SKIPIF--
<?php if (!version_compare(phpversion(), "5.3", '>=')) echo "skip >= PHP 5.3 needed\n"; ?>
--INI--
xdebug.default_enable=1
xdebug.overload_var_dump=1
--FILE--
<?php

namespace foo;

class bar {}

var_dump(get_class(new bar));
?>
--EXPECT--
string(7) "foo\bar"
