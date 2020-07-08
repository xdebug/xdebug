--TEST--
Test for bug #1502: SEND_REF lines are not marked as covered
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
xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE);

include 'bug01502.inc';

$f = new Foo;

$f->sort( [ 5, 2, 5 ] );

$cc = xdebug_get_code_coverage();
ksort($cc);
$result = current(array_slice($cc, 0, 1));

echo "Line 10 covered: ", $result[10] == 1 ? "yes" : "no", "\n";
echo "Line 15 covered: ", $result[15] == 1 ? "yes" : "no", "\n";

xdebug_stop_code_coverage(false);
?>
--EXPECTF--
Line 10 covered: yes
Line 15 covered: yes
