--TEST--
Test for nested function calls (PHP >= 8.0)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.0');
?>
--INI--
xdebug.mode=develop,trace
xdebug.start_with_request=0
xdebug.collect_return=1
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.trace_format=0
xdebug.var_display_max_depth=5
xdebug.var_display_max_children=4
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
%snested-function-calls-php80.php:5:
array(2) {
  [0] =>
  array(4) {
    'function' =>
    string(6) "{main}"
    'file' =>
    string(%d) "%snested-function-calls-php80.php"
    'line' =>
    int(0)
    'params' =>
    array(0) {
    }
  }
  [1] =>
  array(4) {
    'function' =>
    string(1) "a"
    'file' =>
    string(%d) "%snested-function-calls-php80.php"
    'line' =>
    int(17)
    'params' =>
    array(0) {
    }
  }
}
%snested-function-calls-php80.php:5:
array(2) {
  [0] =>
  array(4) {
    'function' =>
    string(6) "{main}"
    'file' =>
    string(%d) "%snested-function-calls-php80.php"
    'line' =>
    int(0)
    'params' =>
    array(0) {
    }
  }
  [1] =>
  array(4) {
    'function' =>
    string(1) "a"
    'file' =>
    string(%d) "%snested-function-calls-php80.php"
    'line' =>
    int(17)
    'params' =>
    array(0) {
    }
  }
}
%snested-function-calls-php80.php:5:
array(2) {
  [0] =>
  array(4) {
    'function' =>
    string(6) "{main}"
    'file' =>
    string(%d) "%snested-function-calls-php80.php"
    'line' =>
    int(0)
    'params' =>
    array(0) {
    }
  }
  [1] =>
  array(4) {
    'function' =>
    string(1) "a"
    'file' =>
    string(%d) "%snested-function-calls-php80.php"
    'line' =>
    int(17)
    'params' =>
    array(0) {
    }
  }
}
aa2ba
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> a() %snested-function-calls-php80.php:17
%w%f %w%d       -> xdebug_get_function_stack() %snested-function-calls-php80.php:5
%w%f %w%d        >=> [0 => ['function' => '{main}', 'file' => '%snested-function-calls-php80.php', 'line' => 0, 'params' => []], 1 => ['function' => 'a', 'file' => '%snested-function-calls-php80.php', 'line' => 17, 'params' => []]]
%w%f %w%d       -> var_dump($value = [0 => ['function' => '{main}', 'file' => '%snested-function-calls-php80.php', 'line' => 0, 'params' => []], 1 => ['function' => 'a', 'file' => '%snested-function-calls-php80.php', 'line' => 17, 'params' => []]]) %snested-function-calls-php80.php:5
%w%f %w%d        >=> NULL
%w%f %w%d      >=> 'a'
%w%f %w%d     -> b($b = 2) %snested-function-calls-php80.php:17
%w%f %w%d      >=> '2b'
%w%f %w%d     -> c($a = 'a', $b = '2b') %snested-function-calls-php80.php:17
%w%f %w%d      >=> '2ba'
%w%f %w%d     -> a() %snested-function-calls-php80.php:17
%w%f %w%d       -> xdebug_get_function_stack() %snested-function-calls-php80.php:5
%w%f %w%d        >=> [0 => ['function' => '{main}', 'file' => '%snested-function-calls-php80.php', 'line' => 0, 'params' => []], 1 => ['function' => 'a', 'file' => '%snested-function-calls-php80.php', 'line' => 17, 'params' => []]]
%w%f %w%d       -> var_dump($value = [0 => ['function' => '{main}', 'file' => '%snested-function-calls-php80.php', 'line' => 0, 'params' => []], 1 => ['function' => 'a', 'file' => '%snested-function-calls-php80.php', 'line' => 17, 'params' => []]]) %snested-function-calls-php80.php:5
%w%f %w%d        >=> NULL
%w%f %w%d      >=> 'a'
%w%f %w%d     -> a() %snested-function-calls-php80.php:17
%w%f %w%d       -> xdebug_get_function_stack() %snested-function-calls-php80.php:5
%w%f %w%d        >=> [0 => ['function' => '{main}', 'file' => '%snested-function-calls-php80.php', 'line' => 0, 'params' => []], 1 => ['function' => 'a', 'file' => '%snested-function-calls-php80.php', 'line' => 17, 'params' => []]]
%w%f %w%d       -> var_dump($value = [0 => ['function' => '{main}', 'file' => '%snested-function-calls-php80.php', 'line' => 0, 'params' => []], 1 => ['function' => 'a', 'file' => '%snested-function-calls-php80.php', 'line' => 17, 'params' => []]]) %snested-function-calls-php80.php:5
%w%f %w%d        >=> NULL
%w%f %w%d      >=> 'a'
%w%f %w%d     -> c($a = 'a', $b = 'a') %snested-function-calls-php80.php:17
%w%f %w%d      >=> 'aa'
%w%f %w%d     -> c($a = '2ba', $b = 'aa') %snested-function-calls-php80.php:17
%w%f %w%d      >=> 'aa2ba'
%w%f %w%d     -> xdebug_stop_trace() %snested-function-calls-php80.php:19
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
