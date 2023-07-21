--TEST--
Test for bug #665: xdebug does not respect display_errors=stderr
--FILE--
<?php
$php = getenv('TEST_PHP_EXECUTABLE') . ' -d log_errors=Off -d xdebug.mode=Off';

$force = '-d xdebug.force_display_errors=0';

$error = '-r ' . escapeshellarg('trigger_error("PASS");');
$exception = '-r ' . escapeshellarg('throw new Exception("PASS");');

$errors_stdout = '-d display_errors=On';
$errors_stderr = '-d display_errors=stderr';
$errors_nowhere = '-d display_errors=Off';

$null = substr(PHP_OS, 0, 3) == 'WIN' ? 'NUL' : '/dev/null';
$output_stdout = "2>$null";
$output_stderr = "2>&1 >$null";

if (`$php $force $error $errors_stdout $output_stdout`) echo "PASS1\n";
if (!`$php $force $error $errors_stderr $output_stdout`) echo "PASS2\n";
if (!`$php $force $error $errors_nowhere $output_stdout`) echo "PASS3\n";

if (!`$php $force $error $errors_stdout $output_stderr`) echo "PASS4\n";
if (`$php $force $error $errors_stderr $output_stderr`) echo "PASS5\n";
if (!`$php $force $error $errors_nowhere $output_stderr`) echo "PASS6\n";

if (`$php $force $exception $errors_stdout $output_stdout`) echo "PASS7\n";
if (!`$php $force $exception $errors_stderr $output_stdout`) echo "PASS8\n";
if (!`$php $force $exception $errors_nowhere $output_stdout`) echo "PASS9\n";

if (!`$php $force $exception $errors_stdout $output_stderr`) echo "PASS10\n";
if (`$php $force $exception $errors_stderr $output_stderr`) echo "PASS11\n";
if (!`$php $force $exception $errors_nowhere $output_stderr`) echo "PASS12\n";
?>
--EXPECT--
PASS1
PASS2
PASS3
PASS4
PASS5
PASS6
PASS7
PASS8
PASS9
PASS10
PASS11
PASS12
