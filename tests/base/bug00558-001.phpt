--TEST--
Test for bug #558: PHP segfaults when running a nested eval
--FILE--
<?php
$any = 'printf("foo\n");';
eval('eval($any);');
?>
DONE
--EXPECT--
foo
DONE
