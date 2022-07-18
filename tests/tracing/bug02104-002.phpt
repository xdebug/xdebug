--TEST--
Test for bug #2104: SensitiveParameter attribute in trace files for internal functions
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('ext hash; PHP >= 8.2');
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

hash_equals('foo', 'key');
hash_hmac('sha256', 'bar', 'baz');

xdebug_stop_trace();
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w               => $tf = '%s' %scapture-trace.inc:17
%w%f %w%d     -> hash_equals($known_string = '[Sensitive Parameter]', $user_string = '[Sensitive Parameter]') %sbug02104-002.php:4
%w%f %w%d     -> hash_hmac($algo = 'sha256', $data = 'bar', $key = '[Sensitive Parameter]') %sbug02104-002.php:5
%w%f %w%d     -> xdebug_stop_trace() %sbug02104-002.php:7
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
