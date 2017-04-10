--TEST--
Test for bug #905: Tracing for generators. (PHP >= 5.5, PHP < 7.1)
--SKIPIF--
<?php
if (!version_compare(phpversion(), "5.5", '>=')) echo "skip >= PHP 5.5 needed\n";
if (!version_compare(phpversion(), "7.1", '<')) echo "skip < PHP 7.1 needed\n";
?>
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=3
xdebug.collect_return=1
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.show_mem_delta=0
xdebug.trace_format=0
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

function gen() {
    yield 'a';
    yield 'b';
    yield 'key' => 'c';
    yield 'd';
    yield 10 => 'e';
    yield 'f';
}
 
foreach (gen() as $key => $value) {
    echo $key, ' => ', $value, "\n";
}

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
0 => a
1 => b
key => c
2 => d
10 => e
11 => f
TRACE START [%d-%d-%d %d:%d:%d]
%w%f %w%d     -> gen() %sbug00905-php5.php:13
%w%f %w%d      >=> (0 => 'a')
%w%f %w%d     -> gen() %sbug00905-php5.php:13
%w%f %w%d      >=> (1 => 'b')
%w%f %w%d     -> gen() %sbug00905-php5.php:13
%w%f %w%d      >=> ('key' => 'c')
%w%f %w%d     -> gen() %sbug00905-php5.php:13
%w%f %w%d      >=> (2 => 'd')
%w%f %w%d     -> gen() %sbug00905-php5.php:13
%w%f %w%d      >=> (10 => 'e')
%w%f %w%d     -> gen() %sbug00905-php5.php:13
%w%f %w%d      >=> (11 => 'f')
%w%f %w%d     -> gen() %sbug00905-php5.php:13
%w%f %w%d     -> xdebug_stop_trace() %sbug00905-php5.php:17
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d]
