--TEST--
Test for nested function calls
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
xdebug.collect_return=1
xdebug.auto_profile=0
xdebug.show_mem_delta=0
--FILE--
<?php
$tf = xdebug_start_trace(tempnam('/tmp', 'xdt'));

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
    string(%d) "/%s/test8.php"
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
    string(%d) "/%s/test8.php"
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
    string(%d) "/%s/test8.php"
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
    string(%d) "/%s/test8.php"
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
    string(%d) "/%s/test8.php"
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
    string(%d) "/%s/test8.php"
    ["line"]=>
    int(17)
    ["params"]=>
    array(0) {
    }
  }
}
aa2ba
TRACE START [%d-%d-%d %d:%d:%d]
    %f      %d     -> a() /%s/test8.php:17
    %f      %d       -> xdebug_get_function_stack() /%s/test8.php:5
                             >=> array (0 => array ('function' => '{main}', 'file' => '/dat/dev/php/xdebug/tests/test8.php', 'line' => 0, 'params' => array ()), 1 => array ('function' => 'a', 'file' => '/dat/dev/php/xdebug/tests/test8.php', 'line' => 17, 'params' => array ()))
    %f      %d       -> var_dump(array (0 => array ('function' => '{main}', 'file' => '/%s/test8.php', 'line' => 0, 'params' => array ()), 1 => array ('function' => 'a', 'file' => '/%s/test8.php', 'line' => 17, 'params' => array ()))) /%s/test8.php:5
                             >=> NULL
                           >=> 'a'
    %f      %d     -> b(2) /%s/test8.php:17
                           >=> '2b'
    %f      %d     -> c('a', '2b') /%s/test8.php:17
                           >=> '2ba'
    %f      %d     -> a() /%s/test8.php:17
    %f      %d       -> xdebug_get_function_stack() /%s/test8.php:5
                             >=> array (0 => array ('function' => '{main}', 'file' => '/dat/dev/php/xdebug/tests/test8.php', 'line' => 0, 'params' => array ()), 1 => array ('function' => 'a', 'file' => '/dat/dev/php/xdebug/tests/test8.php', 'line' => 17, 'params' => array ()))
    %f      %d       -> var_dump(array (0 => array ('function' => '{main}', 'file' => '/%s/test8.php', 'line' => 0, 'params' => array ()), 1 => array ('function' => 'a', 'file' => '/%s/test8.php', 'line' => 17, 'params' => array ()))) /%s/test8.php:5
                             >=> NULL
                           >=> 'a'
    %f      %d     -> a() /%s/test8.php:17
    %f      %d       -> xdebug_get_function_stack() /%s/test8.php:5
                             >=> array (0 => array ('function' => '{main}', 'file' => '/dat/dev/php/xdebug/tests/test8.php', 'line' => 0, 'params' => array ()), 1 => array ('function' => 'a', 'file' => '/dat/dev/php/xdebug/tests/test8.php', 'line' => 17, 'params' => array ()))
    %f      %d       -> var_dump(array (0 => array ('function' => '{main}', 'file' => '/%s/test8.php', 'line' => 0, 'params' => array ()), 1 => array ('function' => 'a', 'file' => '/%s/test8.php', 'line' => 17, 'params' => array ()))) /%s/test8.php:5
                             >=> NULL
                           >=> 'a'
    %f      %d     -> c('a', 'a') /%s/test8.php:17
                           >=> 'aa'
    %f      %d     -> c('2ba', 'aa') /%s/test8.php:17
                           >=> 'aa2ba'
    %f      %d     -> file_get_contents('/tmp/%s') /%s/test8.php:19
