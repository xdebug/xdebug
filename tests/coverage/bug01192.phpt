--TEST--
Test for bug #1192: Dead code analysis does not work for generators with 'return;'
--INI--
xdebug.mode=coverage
--FILE--
<?php
function gen(&$output, $branch = false)
{
	yield;

	if($branch) {
		$output = 'branched';
		return;
	} // This line is never covered.
	$output = 'did not branch';

}

function testGen()
{
	$output = '';
	$gen = gen($output, true);

	while($gen->valid()) {
		$gen->next();
	}
}

xdebug_start_code_coverage (XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE);

testGen();

$c = xdebug_get_code_coverage();
ksort($c);
var_dump($c);
xdebug_stop_code_coverage();
?>
--EXPECTF--
array(1) {
  ["%sbug01192.php"]=>
  array(14) {
    [2]=>
    int(1)
    [4]=>
    int(1)
    [6]=>
    int(1)
    [7]=>
    int(1)
    [8]=>
    int(1)
    [10]=>
    int(-1)
    [12]=>
    int(-1)
    [16]=>
    int(1)
    [17]=>
    int(1)
    [19]=>
    int(1)
    [20]=>
    int(1)
    [22]=>
    int(1)
    [26]=>
    int(1)
    [28]=>
    int(1)
  }
}
