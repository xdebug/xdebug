--TEST--
Test for bug #2104: SensitiveParameter attribute in trace files
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.2');
?>
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.collect_return=0
xdebug.collect_assignments=1
xdebug.trace_format=0
xdebug.var_display_max_data=128
--FILE--
<?php
require_once 'capture-trace.inc';

function takeThemAll(#[SensitiveParameter] string $secret, string $notSecret)
{
}

function takeSplat(string $user, #[SensitiveParameter] string ...$secrets)
{
}

takeThemAll("secret password", "username");
takeSplat("username", "hunter12", "tiger34");

xdebug_stop_trace();
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w               => $tf = '%s' %scapture-trace.inc:17
%w%f %w%d     -> takeThemAll($secret = '[Sensitive Parameter]', $notSecret = 'username') %sbug02104-001.php:12
%w%f %w%d     -> takeSplat($user = 'username', ...$secrets = variadic(0 => '[Sensitive Parameter]', 1 => '[Sensitive Parameter]')) %sbug02104-001.php:13
%w%f %w%d     -> xdebug_stop_trace() %sbug02104-001.php:15
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
