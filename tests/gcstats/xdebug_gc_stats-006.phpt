--TEST--
GC Stats: Start with xdebug_start_gcstats() and filename
--INI--
xdebug.mode=gcstats
xdebug.start_with_request=default
zend.enable_gc=1
--FILE--
<?php
require_once __DIR__ . '/../utils.inc';

var_dump(xdebug_get_gcstats_filename());

$filename = getTmpFile('gcstats.' . uniqid('', TRUE). '.txt');
$actual = xdebug_start_gcstats($filename);

var_dump($actual === $filename);
var_dump(xdebug_get_gcstats_filename());
xdebug_stop_gcstats();
unlink(xdebug_get_gcstats_filename());
?>
--EXPECTF--
bool(false)
bool(true)
string(%d) "%sgcstats.%s.%s.txt"
