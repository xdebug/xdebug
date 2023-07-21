--TEST--
Test for bug #757: XDEBUG_CC_UNUSED does not work with code outside a function (opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('opcache');
?>
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
xdebug_start_code_coverage(XDEBUG_CC_UNUSED);

function f()
{
	$x = 1;
	if ($x) {
		$y = 2;
	} else {
		$y = 3;
	}
	echo $y, "\n";
}

f();

$cc = xdebug_get_code_coverage();
xdebug_stop_code_coverage();
var_dump($cc[__FILE__]);
?>
--EXPECT--
2
array(7) {
  [6]=>
  int(1)
  [7]=>
  int(1)
  [8]=>
  int(1)
  [12]=>
  int(1)
  [13]=>
  int(1)
  [15]=>
  int(1)
  [17]=>
  int(1)
}
