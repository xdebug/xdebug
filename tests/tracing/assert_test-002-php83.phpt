--TEST--
Test for assertion callbacks and description
--INI--
assert.exception=0
error_reporting=E_ALL & ~E_DEPRECATED
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

// Create a handler function
function my_assert_handler ($file, $line, $code, $desc) {
    echo "Assertion Failed:
        File '$file'
        Line '$line'
        Desc '$desc'";
}
// Set up the callback
assert_options (ASSERT_CALLBACK, 'my_assert_handler');

// Make an assertion that should fail
@assert (1 == 2, "One is not two");
echo "\n";

xdebug_stop_trace();
?>
--EXPECTF--
Assertion Failed:
        File '%sassert_test-002.php'
        Line '19'
        Desc 'One is not two'
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> assert_options($option = 1, $value = 1) %sassert_test-002.php:5
%w%f %w%d     -> assert_options($option = 4, $value = 0) %sassert_test-002.php:6
%w%f %w%d     -> assert_options($option = 2, $value = 'my_assert_handler') %sassert_test-002.php:16
%w%f %w%d     -> assert($assertion = FALSE, $description = 'One is not two') %sassert_test-002.php:19
%w%f %w%d       -> my_assert_handler($file = '%sassert_test-002.php', $line = 19, $code = NULL, $desc = 'One is not two') %sassert_test-002.php:19
%w%f %w%d     -> xdebug_stop_trace() %s:%d
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
