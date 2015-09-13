--TEST--
Test for bug #931: Crash with exception in shut-down stage
--FILE--
<?php
session_start();
$_SESSION['test'] = function() { };
echo "DONE";
?>
--EXPECT--
DONE
Fatal error: Uncaught exception 'Exception' with message 'Serialization of 'Closure' is not allowed' in [no active file]:0
Stack trace:
#0 {main}
  thrown in [no active file] on line 0
