--TEST--
Test for bug #800: var_dump(get_class(new foo\bar')) add an extra "\" in class name.
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
