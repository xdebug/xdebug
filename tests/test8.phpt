--TEST--
Test for nested function calls
--INI--
xdebug.enable=1
xdebug.auto_trace=0
--FILE--
<?php
xdebug_start_trace();

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

echo c(c(a(),b(2)), c(a(), a()));

xdebug_dump_function_trace();

?>
--EXPECTF--
array(2) {
  [0]=>
  array(4) {
    ["function"]=>
    string(6) "{main}"
    ["file"]=>
    string(44) "/%s/phpt.%x"
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
    string(44) "/%s/phpt.%x"
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
    string(44) "/%s/phpt.%x"
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
    string(44) "/%s/phpt.%x"
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
    string(44) "/%s/phpt.%x"
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
    string(44) "/%s/phpt.%x"
    ["line"]=>
    int(17)
    ["params"]=>
    array(0) {
    }
  }
}
aa2ba
Function trace:
    %f      %d     -> a() /%s/phpt.%x:17
    %f      %d       -> xdebug_get_function_stack() /%s/phpt.%x:5
    %f      %d       -> var_dump(array (0 => array ('function' => '{main}', 'file' => '/%s/phpt.%x', 'line' => 0, 'params' => array)), 1 => array ('function' => 'a', 'file' => '/%s/phpt.%x', 'line' => 17, 'params' => array)))) /%s/phpt.%x:5
    %f      %d     -> b(2) /%s/phpt.%x:17
    %f      %d     -> c('a', '2b') /%s/phpt.%x:17
    %f      %d     -> a() /%s/phpt.%x:17
    %f      %d       -> xdebug_get_function_stack() /%s/phpt.%x:5
    %f      %d       -> var_dump(array (0 => array ('function' => '{main}', 'file' => '/%s/phpt.%x', 'line' => 0, 'params' => array)), 1 => array ('function' => 'a', 'file' => '/%s/phpt.%x', 'line' => 17, 'params' => array)))) /%s/phpt.%x:5
    %f      %d     -> a() /%s/phpt.%x:17
    %f      %d       -> xdebug_get_function_stack() /%s/phpt.%x:5
    %f      %d       -> var_dump(array (0 => array ('function' => '{main}', 'file' => '/%s/phpt.%x', 'line' => 0, 'params' => array)), 1 => array ('function' => 'a', 'file' => '/%s/phpt.%x', 'line' => 17, 'params' => array)))) /%s/phpt.%x:5
    %f      %d     -> c('a', 'a') /%s/phpt.%x:17
    %f      %d     -> c('2ba', 'aa') /%s/phpt.%x:17
