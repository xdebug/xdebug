--TEST--
Test for bug #693: Cachegrind files not written when filename is very long
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('!win');
if (getenv('CIRCLECI') !== false) die("skip on CircleCI");
?>
--INI--
xdebug.mode=profile
xdebug.start_with_request=default
xdebug.output_dir=/tmp
xdebug.profiler_output_name=cachegrind.01-%r.02-%r.03-%r.04-%r.05-%r.06-%r.07-%r.08-%r.09-%r.10-%r.11-%r.12-%r.13-%r.14-%r.15-%r.16-%r.17-%r.18-%r.19-%r.20-%r.21-%r.22-%r.23-%r.24-%r.25-%r.26-%r.27-%r.28-%r.29-%r.30
--FILE--
<?php
$file = xdebug_get_profiler_filename();
var_dump($file === false);
var_dump(file_exists($file));
var_dump($file);
if ($file) {
	unlink($file);
}
?>
--EXPECTF--
bool(false)
bool(true)
string(%d) "%scachegrind.01-%s.02-%s.03-%s.04-%s.05-%s.06-%s.07-%s.08-%s.09-%s.10-%s.11-%s.12-%s.13-%s.14-%s.15-%s.16-%s.17-%s.18-%s.19-%s.20-%s.21-%s.22-%s.23-%s.24-%s"
