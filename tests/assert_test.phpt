--TEST--
Test for assertion callbacks
--INI--
xdebug.enable=1
xdebug.auto_trace=1
xdebug.collect_params=1
--FILE--
<?php
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
xdebug_dump_function_trace();
?>
--EXPECTF--
Assertion Failed:
        File '/%s/assert_test.php'
        Line '19'
        Code ''
Function trace:
    %f      %d   -> {main}() /%s/assert_test.php:0
    %f      %d     -> assert_options(1, 1) /%s/assert_test.php:3
    %f      %d     -> assert_options(4, 0) /%s/assert_test.php:4
    %f      %d     -> assert_options(5, 1) /%s/assert_test.php:5
    %f      %d     -> assert_options(2, 'my_assert_handler') /%s/assert_test.php:16
    %f      %d     -> assert(FALSE) /%s/assert_test.php:19
    %f      %d       -> my_assert_handler('/%s/assert_test.php', 19, '') /%s/assert_test.php:19
