--TEST--
Test for bug #1258: ensure case statements are covered (< PHP 7.0)
--SKIPIF--
<?php if (!version_compare(phpversion(), "7.0", '<')) echo "skip < PHP 7.0 needed\n"; ?>
--FILE--
<?php
$foo = ['bar', 'baz', 'qux', 'quux'];
xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE);
foreach ($foo as $k => $v) {
    switch ($v) {
        case 'bar':
            echo "bar\n";
            break;
        case 'baz':
            echo "baz\n";
            break;
        case 'qux':
            echo "qux\n";
            break;
        default:
            echo "default\n";
            break;
    }
}
$cc = xdebug_get_code_coverage();
xdebug_stop_code_coverage();
var_dump($cc);
?>
--EXPECTF--
bar
baz
qux
default
array(1) {
  ["%sbug01258-php5.php"]=>
  array(16) {
    [4]=>
    int(1)
    [6]=>
    int(1)
    [7]=>
    int(1)
    [8]=>
    int(1)
    [9]=>
    int(1)
    [10]=>
    int(1)
    [11]=>
    int(1)
    [12]=>
    int(1)
    [13]=>
    int(1)
    [14]=>
    int(1)
    [15]=>
    int(1)
    [16]=>
    int(1)
    [17]=>
    int(1)
    [18]=>
    int(1)
    [19]=>
    int(1)
    [20]=>
    int(1)
  }
}
