--TEST--
Test for bug #631: Summary not written when script ended with "exit()".
--INI--
xdebug.profiler_enable=1
--FILE--
<?php
function capture() {
	echo file_get_contents(xdebug_get_profiler_filename());
}

register_shutdown_function('capture');
strlen("5");
exit();
?>
--EXPECTF--
version: 1
creator: xdebug 2.%s
cmd: %sbug00631.php
part: 1
positions: line

events: Time

fl=php:internal
fn=php::register_shutdown_function
%d %d

fl=php:internal
fn=php::strlen
%d %d

fl=%sbug00631.php
fn={main}

summary: %d

%d %d
cfn=php::register_shutdown_function
calls=1 0 0
%d %d
cfn=php::strlen
calls=1 0 0
%d %d

fl=php:internal
fn=php::xdebug_get_profiler_filename
%d %d
