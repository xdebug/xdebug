--TEST--
Test for bug #1851: Paths are not counted as coveraged with loops calling function
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('ext pdo_sqlite');
?>
--INI--
xdebug.mode=coverage
xdebug.start_with_request=trigger
--FILE--
<?php
include 'dump-branch-coverage.inc';

xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE | XDEBUG_CC_BRANCH_CHECK);

require dirname(__FILE__) . '/bug01851.inc';

xdebug_stop_code_coverage(false);
$c = xdebug_get_code_coverage();
dump_branch_coverage($c);

?>
--EXPECTF--
%a
- paths
  - 0 %d %d %d %d: HIT
  - 0 %d %d:  X 
  - 0 %d:  X 
%a
