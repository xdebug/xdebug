--TEST--
Test for bug #1092: Dead code analysis does not work for generators with 'return;' (>= PHP 5.5, < PHP 7.0)
--SKIPIF--
<?php if (!version_compare(phpversion(), "5.5", '>=')) echo "skip >= PHP 5.5 needed\n"; ?>
<?php if (!version_compare(phpversion(), "7.0", '<')) echo "skip < PHP 7.0 needed\n"; ?>
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
  ["%sbug01192-php5.php"]=>
  array(15) {
    [4]=>
    int(1)
    [6]=>
    int(1)
    [7]=>
    int(1)
    [8]=>
    int(1)
    [9]=>
    int(-2)
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
    [21]=>
    int(1)
    [22]=>
    int(1)
    [26]=>
    int(1)
    [28]=>
    int(1)
  }
}
