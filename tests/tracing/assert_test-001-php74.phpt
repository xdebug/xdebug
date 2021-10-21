--TEST--
Test for assertion callbacks (PHP < 8.0)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP < 8.0');
?>
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.trace_format=0
--FILE--
<?php
require_once 'capture-trace.inc';

// Active assert and make it quiet
assert_options (ASSERT_ACTIVE, 1);
assert_options (ASSERT_WARNING, 0);
assert_options (ASSERT_QUIET_EVAL, 1);

// Create a handler function
function my_assert_handler ($file, $line, $code) {
    echo "Assertion Failed:
        File '$file'
        Line '$line'
        Code '$code'";
}

// Set up the callback
assert_options (ASSERT_CALLBACK, 'my_assert_handler');

// Make an assertion that should fail
@assert ('1==2');
echo "\n";

xdebug_stop_trace();
?>
--EXPECTF--
Assertion Failed:
        File '%sassert_test-001-php74.php'
        Line '21'
        Code '1==2'
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> assert_options($what = 1, $value = 1) %sassert_test-001-php74.php:5
%w%f %w%d     -> assert_options($what = 4, $value = 0) %sassert_test-001-php74.php:6
%w%f %w%d     -> assert_options($what = 5, $value = 1) %sassert_test-001-php74.php:7
%w%f %w%d     -> assert_options($what = 2, $value = 'my_assert_handler') %sassert_test-001-php74.php:18
%w%f %w%d     -> assert($assertion = '1==2') %sassert_test-001-php74.php:21
%w%f %w%d       -> %r({internal eval}\(\))|(assert\('1==2'\))%r %sassert_test-001-php74.php:21
%w%f %w%d       -> my_assert_handler($file = '%sassert_test-001-php74.php', $line = 21, $code = '1==2') %sassert_test-001-php74.php:21
%w%f %w%d     -> xdebug_stop_trace() %s:%d
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
