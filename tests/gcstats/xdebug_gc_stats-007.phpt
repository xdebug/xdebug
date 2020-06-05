--TEST--
GC Stats: userland statistic functions
--INI--
xdebug.mode=gcstats
xdebug.start_with_request=default
zend.enable_gc=1
--FILE--
<?php

for ($i = 0; $i < 100; $i++) {
	$a = new stdClass();
	$b = new stdClass();
	$b->a = $a;
	$a->b = $b;
	unset($a, $b);
}
gc_collect_cycles();

printf("runs: %d\n", xdebug_get_gc_run_count());
printf("collected: %d\n", xdebug_get_gc_total_collected_roots());
?>
--EXPECTF--
runs: 1
collected: 200
