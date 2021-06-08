--TEST--
Test with Code Coverage with unused lines
--INI--
xdebug.mode=coverage
xdebug.trace_options=0
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.dump_globals=0
xdebug.trace_format=0
--FILE--
<?php
xdebug_start_code_coverage(true);

include 'coverage3.inc';

a(1);

xdebug_stop_code_coverage(false);
$cc = xdebug_get_code_coverage();
ksort($cc);
var_dump($cc);
?>
--EXPECTF--
array(2) {
  ["%scoverage3.inc"]=>
  array(7) {
    [2]=>
    int(1)
    [4]=>
    int(1)
    [6]=>
    int(-1)
    [9]=>
    int(1)
    [11]=>
    int(1)
    [13]=>
    int(-1)
    [14]=>
    int(1)
  }
  ["%scoverage3.php"]=>
  array(3) {
    [4]=>
    int(1)
    [6]=>
    int(1)
    [8]=>
    int(1)
  }
}
