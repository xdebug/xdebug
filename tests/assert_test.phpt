--TEST--
Test for assertion callbacks
--INI--
xdebug.enable=1
xdebug.auto_trace=1
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
    0.0000      35352   -> {main}() /%s/assert_test.php:0
    0.0001      35400     -> assert_options(1, 1) /%s/assert_test.php:3
    0.0006      35400     -> assert_options(4, 0) /%s/assert_test.php:4
    0.0006      35400     -> assert_options(5, 1) /%s/assert_test.php:5
    0.0006      35400     -> assert_options(2, 'my_assert_handler') /%s/assert_test.php:16
    0.0006      35352     -> assert(FALSE) /%s/assert_test.php:19
    0.0009      35504       -> my_assert_handler('/%s/assert_test.php', 19, '') /%s/assert_test.php:19
