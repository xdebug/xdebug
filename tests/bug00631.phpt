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
version: 0.9.6
cmd: /home/derick/dev/php/xdebug/trunk/tests/bug00XXX.php
part: 1

events: Time

fl=php:internal
fn=php::register_shutdown_function
%d %d

fl=php:internal
fn=php::strlen
%d %d

fl=/home/derick/dev/php/xdebug/trunk/tests/bug00XXX.php
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
