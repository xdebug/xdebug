--TEST--
Test for nested function calls
--INI--
xdebug.mode=develop,trace
xdebug.start_with_request=no
xdebug.collect_return=1
xdebug.collect_assignments=0
xdebug.trace_format=0
xdebug.var_display_max_depth=5
xdebug.var_display_max_children=6
--FILE--
<?php
require_once 'capture-trace.inc';

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
?>
--EXPECTF--
%snested-function-calls.php:5:
array(2) {
  [0] =>
  array(6) {
    'time' =>
    double(%f)
    'memory' =>
    int(%d)
    'function' =>
    string(6) "{main}"
    'file' =>
    string(%d) "%snested-function-calls.php"
    'line' =>
    int(0)
    'params' =>
    array(0) {
    }
  }
  [1] =>
  array(6) {
    'time' =>
    double(%f)
    'memory' =>
    int(%d)
    'function' =>
    string(1) "a"
    'file' =>
    string(%d) "%snested-function-calls.php"
    'line' =>
    int(17)
    'params' =>
    array(0) {
    }
  }
}
%snested-function-calls.php:5:
array(2) {
  [0] =>
  array(6) {
    'time' =>
    double(%f)
    'memory' =>
    int(%d)
    'function' =>
    string(6) "{main}"
    'file' =>
    string(%d) "%snested-function-calls.php"
    'line' =>
    int(0)
    'params' =>
    array(0) {
    }
  }
  [1] =>
  array(6) {
    'time' =>
    double(%f)
    'memory' =>
    int(%d)
    'function' =>
    string(1) "a"
    'file' =>
    string(%d) "%snested-function-calls.php"
    'line' =>
    int(17)
    'params' =>
    array(0) {
    }
  }
}
%snested-function-calls.php:5:
array(2) {
  [0] =>
  array(6) {
    'time' =>
    double(%f)
    'memory' =>
    int(%d)
    'function' =>
    string(6) "{main}"
    'file' =>
    string(%d) "%snested-function-calls.php"
    'line' =>
    int(0)
    'params' =>
    array(0) {
    }
  }
  [1] =>
  array(6) {
    'time' =>
    double(%f)
    'memory' =>
    int(%d)
    'function' =>
    string(1) "a"
    'file' =>
    string(%d) "%snested-function-calls.php"
    'line' =>
    int(17)
    'params' =>
    array(0) {
    }
  }
}
aa2ba
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> a() %snested-function-calls.php:17
%w%f %w%d       -> xdebug_get_function_stack() %snested-function-calls.php:5
%w%f %w%d        >=> [0 => ['time' => %f, 'memory' => %d, 'function' => '{main}', 'file' => '%snested-function-calls.php', 'line' => 0, 'params' => []], 1 => ['time' => %f, 'memory' => %d, 'function' => 'a', 'file' => '%snested-function-calls.php', 'line' => 17, 'params' => []]]
%w%f %w%d       -> var_dump($value = [0 => ['time' => %f, 'memory' => %d, 'function' => '{main}', 'file' => '%snested-function-calls.php', 'line' => 0, 'params' => []], 1 => ['time' => %f, 'memory' => %d, 'function' => 'a', 'file' => '%snested-function-calls.php', 'line' => 17, 'params' => []]]) %snested-function-calls.php:5
%w%f %w%d        >=> NULL
%w%f %w%d      >=> 'a'
%w%f %w%d     -> b($b = 2) %snested-function-calls.php:17
%w%f %w%d      >=> '2b'
%w%f %w%d     -> c($a = 'a', $b = '2b') %snested-function-calls.php:17
%w%f %w%d      >=> '2ba'
%w%f %w%d     -> a() %snested-function-calls.php:17
%w%f %w%d       -> xdebug_get_function_stack() %snested-function-calls.php:5
%w%f %w%d        >=> [0 => ['time' => %f, 'memory' => %d, 'function' => '{main}', 'file' => '%snested-function-calls.php', 'line' => 0, 'params' => []], 1 => ['time' => %f, 'memory' => %d, 'function' => 'a', 'file' => '%snested-function-calls.php', 'line' => 17, 'params' => []]]
%w%f %w%d       -> var_dump($value = [0 => ['time' => %f, 'memory' => %d, 'function' => '{main}', 'file' => '%snested-function-calls.php', 'line' => 0, 'params' => []], 1 => ['time' => %f, 'memory' => %d, 'function' => 'a', 'file' => '%snested-function-calls.php', 'line' => 17, 'params' => []]]) %snested-function-calls.php:5
%w%f %w%d        >=> NULL
%w%f %w%d      >=> 'a'
%w%f %w%d     -> a() %snested-function-calls.php:17
%w%f %w%d       -> xdebug_get_function_stack() %snested-function-calls.php:5
%w%f %w%d        >=> [0 => ['time' => %f, 'memory' => %d, 'function' => '{main}', 'file' => '%snested-function-calls.php', 'line' => 0, 'params' => []], 1 => ['time' => %f, 'memory' => %d, 'function' => 'a', 'file' => '%snested-function-calls.php', 'line' => 17, 'params' => []]]
%w%f %w%d       -> var_dump($value = [0 => ['time' => %f, 'memory' => %d, 'function' => '{main}', 'file' => '%snested-function-calls.php', 'line' => 0, 'params' => []], 1 => ['time' => %f, 'memory' => %d, 'function' => 'a', 'file' => '%snested-function-calls.php', 'line' => 17, 'params' => []]]) %snested-function-calls.php:5
%w%f %w%d        >=> NULL
%w%f %w%d      >=> 'a'
%w%f %w%d     -> c($a = 'a', $b = 'a') %snested-function-calls.php:17
%w%f %w%d      >=> 'aa'
%w%f %w%d     -> c($a = '2ba', $b = 'aa') %snested-function-calls.php:17
%w%f %w%d      >=> 'aa2ba'
%w%f %w%d     -> xdebug_stop_trace() %snested-function-calls.php:19
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
