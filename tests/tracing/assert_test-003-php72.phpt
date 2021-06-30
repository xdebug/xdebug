--TEST--
Test for assertion callbacks and exception (PHP < 8.0)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP < 8.0');
?>
--INI--
xdebug.mode=trace
xdebug.start_with_request=0
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.trace_format=0
zend.assertions=1
assert.exception=1
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

// Active assert and make it quiet
assert_options (ASSERT_ACTIVE, 1);
assert_options (ASSERT_WARNING, 0);
assert_options (ASSERT_QUIET_EVAL, 1);

// Create a handler function
function my_assert_handler ($file, $line, $code, $desc) {
    echo "Assertion Failed:
        File '$file'
        Line '$line'
        Code '$code'
        Desc '$desc'";
}
// Set up the callback
assert_options (ASSERT_CALLBACK, 'my_assert_handler');

// Make an assertion that should fail
try
{
	@assert ('1==2', "One is not two");
} catch (AssertionError $e )
{
	echo "\n", $e->getMessage(), "\n";
}
echo "\n";
echo file_get_contents($tf);
xdebug_stop_trace();
unlink($tf);
?>
--EXPECTF--
Assertion Failed:
        File '%sassert_test-003-php72.php'
        Line '23'
        Code '1==2'
        Desc 'One is not two'
One is not two

TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> assert_options($what = 1, $value = 1) %sassert_test-003-php72.php:5
%w%f %w%d     -> assert_options($what = 4, $value = 0) %sassert_test-003-php72.php:6
%w%f %w%d     -> assert_options($what = 5, $value = 1) %sassert_test-003-php72.php:7
%w%f %w%d     -> assert_options($what = 2, $value = 'my_assert_handler') %sassert_test-003-php72.php:18
%w%f %w%d     -> assert($assertion = '1==2', $description = 'One is not two') %sassert_test-003-php72.php:23
%w%f %w%d       -> %r({internal eval}\(\))|(assert\('1==2'\))%r %sassert_test-003-php72.php:23
%w%f %w%d       -> my_assert_handler($file = '%sassert_test-003-php72.php', $line = 23, $code = '1==2', $desc = 'One is not two') %sassert_test-003-php72.php:23
%w%f %w%d     -> Error->getMessage() %sassert_test-003-php72.php:26
%w%f %w%d     -> file_get_contents($filename = '%s') %sassert_test-003-php72.php:29
