--TEST--
Test with eval()
--INI--
xdebug.enable=1
xdebug.auto_trace=0
--FILE--
<?php
xdebug_start_trace();

function bar()
{
	return "bar";
}

function foo()
{
	return bar();
}

foo();

eval("\$foo = foo();\nbar();\nfoo();\n");
echo $foo;
var_dump(xdebug_get_function_trace());
?>
--EXPECTF--
bararray(8) {
  [0]=>
  array(6) {
    ["function"]=>
    string(3) "foo"
    ["file"]=>
    string(%d) "/%s/get_function_trace.php"
    ["line"]=>
    int(14)
    ["time_index"]=>
    int(0)
    ["memory_usage"]=>
    int(%d)
    ["params"]=>
    array(0) {
    }
  }
  [1]=>
  array(6) {
    ["function"]=>
    string(3) "bar"
    ["file"]=>
    string(%d) "/%s/get_function_trace.php"
    ["line"]=>
    int(11)
    ["time_index"]=>
    int(0)
    ["memory_usage"]=>
    int(%d)
    ["params"]=>
    array(0) {
    }
  }
  [2]=>
  array(6) {
    ["function"]=>
    string(6) "{eval}"
    ["file"]=>
    string(48) "/%s/get_function_trace.php"
    ["line"]=>
    int(16)
    ["time_index"]=>
    int(0)
    ["memory_usage"]=>
    int(%d)
    ["params"]=>
    array(1) {
      [1]=>
      string(28) "$foo = foo();
bar();
foo();
"
    }
  }
  [3]=>
  array(6) {
    ["function"]=>
    string(3) "foo"
    ["file"]=>
    string(%d) "/%s/get_function_trace.php(16) : eval()'d code"
    ["line"]=>
    int(1)
    ["time_index"]=>
    int(0)
    ["memory_usage"]=>
    int(%d)
    ["params"]=>
    array(0) {
    }
  }
  [4]=>
  array(6) {
    ["function"]=>
    string(3) "bar"
    ["file"]=>
    string(%d) "/%s/get_function_trace.php"
    ["line"]=>
    int(11)
    ["time_index"]=>
    int(0)
    ["memory_usage"]=>
    int(%d)
    ["params"]=>
    array(0) {
    }
  }
  [5]=>
  array(6) {
    ["function"]=>
    string(3) "bar"
    ["file"]=>
    string(%d) "/%s/get_function_trace.php(16) : eval()'d code"
    ["line"]=>
    int(2)
    ["time_index"]=>
    int(0)
    ["memory_usage"]=>
    int(%d)
    ["params"]=>
    array(0) {
    }
  }
  [6]=>
  array(6) {
    ["function"]=>
    string(3) "foo"
    ["file"]=>
    string(%d) "/%s/get_function_trace.php(16) : eval()'d code"
    ["line"]=>
    int(3)
    ["time_index"]=>
    int(0)
    ["memory_usage"]=>
    int(%d)
    ["params"]=>
    array(0) {
    }
  }
  [7]=>
  array(6) {
    ["function"]=>
    string(3) "bar"
    ["file"]=>
    string(%d) "/%s/get_function_trace.php"
    ["line"]=>
    int(11)
    ["time_index"]=>
    int(0)
    ["memory_usage"]=>
    int(%d)
    ["params"]=>
    array(0) {
    }
  }
}
