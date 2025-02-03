--TEST--
Test for bug #1674: Inconsistent Path & Branch Coverage Reported
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
?>
--INI--
xdebug.mode=coverage
--FILE--
<?php
declare(strict_types=1);

require __DIR__ . '/../utils.inc';

use Xdebug\SampleClass;
use Xdebug\SampleStaticClass;

\xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE | XDEBUG_CC_BRANCH_CHECK);
require_once __DIR__ . '/bug01674-SampleStaticClass.inc';
require_once __DIR__ . '/bug01674-SampleClass.inc';
\xdebug_stop_code_coverage(true);

$resultPrinter = static function (array $results, int $run) {
    foreach ($results as $file => $result) {
        if (\strpos($file, 'bug01674-SampleStaticClass.inc') === false && \strpos($file, 'bug01674-SampleClass.inc') === false) {
            continue;
        }

        $expected = 9;
        if (\strpos($file, 'SampleClass.php')) {
            $expected = 5;
        }

        \printf(
            "Run %2d: Found %d functions in file %s, expected %d\n",
            $run,
            \count($result['functions']),
            $file,
            $expected
        );
		$functions = array_keys($result['functions']);
		sort($functions);
		\print_r($functions);
    }

    print PHP_EOL;
};

\xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE | XDEBUG_CC_BRANCH_CHECK);
$object1 = new SampleClass();
assert(SampleStaticClass::getField0() === '0');
assert(SampleStaticClass::getField1() === '1');
assert(SampleStaticClass::getField2() === '2');
assert($object1->getField0NonStatic() === '0');
assert($object1->getField1NonStatic() === '1');
assert($object1->getField2NonStatic() === '2');
$results = \xdebug_get_code_coverage();
$resultPrinter(x_sort($results), 0);
\xdebug_stop_code_coverage(true);

\xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE | XDEBUG_CC_BRANCH_CHECK);
assert(SampleStaticClass::getField1() === '1');
assert($object1->getField1NonStatic() === '1');
$results = \xdebug_get_code_coverage();
$resultPrinter(x_sort($results), 1);
\xdebug_stop_code_coverage(true);
?>
--EXPECTF--
Run  0: Found 5 functions in file %sbug01674-SampleClass.inc, expected 9
Array
(
    [0] => Xdebug\SampleClass->getField0NonStatic
    [1] => Xdebug\SampleClass->getField1NonStatic
    [2] => Xdebug\SampleClass->getField2NonStatic
    [3] => Xdebug\SampleClass->getField3NonStatic
    [4] => {main}
)
Run  0: Found 9 functions in file %sbug01674-SampleStaticClass.inc, expected 9
Array
(
    [0] => Xdebug\SampleStaticClass->getField0
    [1] => Xdebug\SampleStaticClass->getField0NonStatic
    [2] => Xdebug\SampleStaticClass->getField1
    [3] => Xdebug\SampleStaticClass->getField1NonStatic
    [4] => Xdebug\SampleStaticClass->getField2
    [5] => Xdebug\SampleStaticClass->getField2NonStatic
    [6] => Xdebug\SampleStaticClass->getField3
    [7] => Xdebug\SampleStaticClass->getField3NonStatic
    [8] => {main}
)

Run  1: Found 5 functions in file %sbug01674-SampleClass.inc, expected 9
Array
(
    [0] => Xdebug\SampleClass->getField0NonStatic
    [1] => Xdebug\SampleClass->getField1NonStatic
    [2] => Xdebug\SampleClass->getField2NonStatic
    [3] => Xdebug\SampleClass->getField3NonStatic
    [4] => {main}
)
Run  1: Found 9 functions in file %sbug01674-SampleStaticClass.inc, expected 9
Array
(
    [0] => Xdebug\SampleStaticClass->getField0
    [1] => Xdebug\SampleStaticClass->getField0NonStatic
    [2] => Xdebug\SampleStaticClass->getField1
    [3] => Xdebug\SampleStaticClass->getField1NonStatic
    [4] => Xdebug\SampleStaticClass->getField2
    [5] => Xdebug\SampleStaticClass->getField2NonStatic
    [6] => Xdebug\SampleStaticClass->getField3
    [7] => Xdebug\SampleStaticClass->getField3NonStatic
    [8] => {main}
)
