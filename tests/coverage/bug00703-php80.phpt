--TEST--
Test for bug #703: Line in heredoc marked as not executed (>= PHP 8.0)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.0');
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
$file = 'bug00703.inc';
$pathname = stream_resolve_include_path( $file );

xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE);

require $pathname;

$cc = xdebug_get_code_coverage();
xdebug_stop_code_coverage(false);

var_dump( $cc[$pathname] );

?>
--EXPECTF--
array(6) {
  [3]=>
  int(1)
  [5]=>
  int(1)
  [6]=>
  int(1)
  [10]=>
  int(1)
  [11]=>
  int(1)
  [15]=>
  int(1)
}
