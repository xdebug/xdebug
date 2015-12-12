--TEST--
Test for nested function calls
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=3
xdebug.collect_return=1
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.show_mem_delta=0
xdebug.trace_format=0
xdebug.var_display_max_depth=5
xdebug.var_display_max_children=4
xdebug.overload_var_dump=0
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

function a () {
	var_dump (xdebug_get_function_stack());
	return 'a';
}

function b ($b) {
	return $b.'b';
}

function c ($a, $b) {
	return $b.$a;
}

echo c(c(a(),b(2)), c(a(), a())), "\n";

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
array(2) {
  [0]=>
  array(4) {
    ["function"]=>
    string(6) "{main}"
    ["file"]=>
    string(%d) "%stest8.php"
    ["line"]=>
    int(0)
    ["params"]=>
    array(0) {
    }
  }
  [1]=>
  array(4) {
    ["function"]=>
    string(1) "a"
    ["file"]=>
    string(%d) "%stest8.php"
    ["line"]=>
    int(17)
    ["params"]=>
    array(0) {
    }
  }
}
array(2) {
  [0]=>
  array(4) {
    ["function"]=>
    string(6) "{main}"
    ["file"]=>
    string(%d) "%stest8.php"
    ["line"]=>
    int(0)
    ["params"]=>
    array(0) {
    }
  }
  [1]=>
  array(4) {
    ["function"]=>
    string(1) "a"
    ["file"]=>
    string(%d) "%stest8.php"
    ["line"]=>
    int(17)
    ["params"]=>
    array(0) {
    }
  }
}
array(2) {
  [0]=>
  array(4) {
    ["function"]=>
    string(6) "{main}"
    ["file"]=>
    string(%d) "%stest8.php"
    ["line"]=>
    int(0)
    ["params"]=>
    array(0) {
    }
  }
  [1]=>
  array(4) {
    ["function"]=>
    string(1) "a"
    ["file"]=>
    string(%d) "%stest8.php"
    ["line"]=>
    int(17)
    ["params"]=>
    array(0) {
    }
  }
}
aa2ba
TRACE START [%d-%d-%d %d:%d:%d]
%w%f %w%d     -> a() %stest8.php:17
%w%f %w%d       -> xdebug_get_function_stack() %stest8.php:5
%w%f %w%d        >=> array (0 => array ('function' => '{main}', 'file' => '%stest8.php', 'line' => 0, 'params' => array ()), 1 => array ('function' => 'a', 'file' => '%stest8.php', 'line' => 17, 'params' => array ()))
%w%f %w%d       -> var_dump(array (0 => array ('function' => '{main}', 'file' => '%stest8.php', 'line' => 0, 'params' => array ()), 1 => array ('function' => 'a', 'file' => '%stest8.php', 'line' => 17, 'params' => array ()))) %stest8.php:5
%w%f %w%d        >=> NULL
%w%f %w%d      >=> 'a'
%w%f %w%d     -> b(2) %stest8.php:17
%w%f %w%d      >=> '2b'
%w%f %w%d     -> c('a', '2b') %stest8.php:17
%w%f %w%d      >=> '2ba'
%w%f %w%d     -> a() %stest8.php:17
%w%f %w%d       -> xdebug_get_function_stack() %stest8.php:5
%w%f %w%d        >=> array (0 => array ('function' => '{main}', 'file' => '%stest8.php', 'line' => 0, 'params' => array ()), 1 => array ('function' => 'a', 'file' => '%stest8.php', 'line' => 17, 'params' => array ()))
%w%f %w%d       -> var_dump(array (0 => array ('function' => '{main}', 'file' => '%stest8.php', 'line' => 0, 'params' => array ()), 1 => array ('function' => 'a', 'file' => '%stest8.php', 'line' => 17, 'params' => array ()))) %stest8.php:5
%w%f %w%d        >=> NULL
%w%f %w%d      >=> 'a'
%w%f %w%d     -> a() %stest8.php:17
%w%f %w%d       -> xdebug_get_function_stack() %stest8.php:5
%w%f %w%d        >=> array (0 => array ('function' => '{main}', 'file' => '%stest8.php', 'line' => 0, 'params' => array ()), 1 => array ('function' => 'a', 'file' => '%stest8.php', 'line' => 17, 'params' => array ()))
%w%f %w%d       -> var_dump(array (0 => array ('function' => '{main}', 'file' => '%stest8.php', 'line' => 0, 'params' => array ()), 1 => array ('function' => 'a', 'file' => '%stest8.php', 'line' => 17, 'params' => array ()))) %stest8.php:5
%w%f %w%d        >=> NULL
%w%f %w%d      >=> 'a'
%w%f %w%d     -> c('a', 'a') %stest8.php:17
%w%f %w%d      >=> 'aa'
%w%f %w%d     -> c('2ba', 'aa') %stest8.php:17
%w%f %w%d      >=> 'aa2ba'
%w%f %w%d     -> xdebug_stop_trace() %stest8.php:19
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d]
