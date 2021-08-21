--TEST--
Test for bug #2009: xdebug_stop_code_coverage's argument has type mismatch
--INI--
xdebug.mode=coverage
--FILE--
<?php
declare(strict_types=1);
xdebug_start_code_coverage();
xdebug_stop_code_coverage(true);
echo "---DONE---\n";
?>
--EXPECTF--
---DONE---
