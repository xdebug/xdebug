--TEST--
GC Stats: No memleak, return empty runs
--INI--
xdebug.mode=gcstats
xdebug.start_with_request=yes
zend.enable_gc=1
--FILE--
<?php
echo file_get_contents(xdebug_get_gcstats_filename());
xdebug_stop_gcstats();
unlink(xdebug_get_gcstats_filename());
?>
--EXPECTF--
Garbage Collection Report
version: 1
creator: xdebug %d.%s (PHP %s)

Collected | Efficiency% | Duration | Memory Before | Memory After | Reduction% | Function
----------+-------------+----------+---------------+--------------+------------+---------
