--TEST--
Test for bug #631: Summary not written when script ended with "exit()"
--INI--
xdebug.mode=profile
xdebug.start_with_request=1
--FILE--
<?php
$filename = xdebug_get_profiler_filename();

function capture() {
	global $filename;
	echo file_get_contents($filename);
	unlink($filename);
}

register_shutdown_function('capture');
strrev("5");
exit();
?>
--EXPECTF--
version: 1
creator: xdebug %d.%s (PHP %s)
cmd: %sbug00631.php
part: 1
positions: line

events: Time_(µs) Memory_(bytes)

fl=(1) php:internal
fn=(1) php::xdebug_get_profiler_filename
%d %d %d

fl=(1)
fn=(2) php::register_shutdown_function
%d %d %i

fl=(1)
fn=(3) php::strrev
%d %d %i

fl=(2) %sbug00631.php
fn=(4) {main}
%d %d %i
cfl=(1)
cfn=(1)
calls=1 0 0
%d %d %i
cfl=(1)
cfn=(2)
calls=1 0 0
%d %d %i
cfl=(1)
cfn=(3)
calls=1 0 0
%d %d %i

summary: %d %i
