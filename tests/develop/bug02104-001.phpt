--TEST--
Test for bug #2104: SensitiveParameter attribute in stack traces
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.2');
?>
--INI--
html_errors=0
xdebug.mode=develop
--FILE--
<?php
function takeThemAll(#[SensitiveParameter] string $secret, string $notSecret)
{
	xdebug_print_function_stack();
}

function takeSplat(string $user, #[SensitiveParameter] string ...$secrets)
{
	xdebug_print_function_stack();
}

takeThemAll("secret password", "username");
takeSplat("username", "hunter12", "tiger34");
?>
--EXPECTF--
Xdebug: user triggered in %sbug02104-001.php on line 4

Call Stack:
%w%f %w%d   1. {main}() %sbug02104-001.php:0
%w%f %w%d   2. takeThemAll($secret = '[Sensitive Parameter]', $notSecret = 'username') %sbug02104-001.php:12
%w%f %w%d   3. xdebug_print_function_stack() %sbug02104-001.php:4


Xdebug: user triggered in %sbug02104-001.php on line 9

Call Stack:
%w%f %w%d   1. {main}() %sbug02104-001.php:0
%w%f %w%d   2. takeSplat($user = 'username', ...$secrets = variadic('[Sensitive Parameter]', '[Sensitive Parameter]')) %sbug02104-001.php:13
%w%f %w%d   3. xdebug_print_function_stack() %sbug02104-001.php:9
