--TEST--
Test for assertion callbacks
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.show_mem_delta=0
xdebug.trace_format=0
--FILE--
<?php
$tf = xdebug_start_trace('/tmp/'. uniqid('xdt', TRUE));

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
assert (1==2);
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
Assertion Failed:
        File '/%s/assert_test.php'
        Line '21'
        Code ''
TRACE START [%d-%d-%d %d:%d:%d]
    %f      %d     -> assert_options(1, 1) /%s/assert_test.php:5
    %f      %d     -> assert_options(4, 0) /%s/assert_test.php:6
    %f      %d     -> assert_options(5, 1) /%s/assert_test.php:7
    %f      %d     -> assert_options(2, 'my_assert_handler') /%s/assert_test.php:18
    %f      %d     -> assert(FALSE) /%s/assert_test.php:21
    %f      %d       -> my_assert_handler('/%s/assert_test.php', 21, '') /%s/assert_test.php:21
    %f      %d     -> file_get_contents('/tmp/%s') /%s/assert_test.php:22
