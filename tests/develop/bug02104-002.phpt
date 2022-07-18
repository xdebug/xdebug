--TEST--
Test for bug #2104: SensitiveParameter attribute in stack traces for internal functions
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('ext hash; PHP >= 8.2');
?>
--INI--
html_errors=0
xdebug.mode=develop
--FILE--
<?php
hash_hmac('foo', 'bar', 'baz');
?>
--EXPECTF--
Fatal error: Uncaught ValueError: hash_hmac(): Argument #1 ($algo) must be a valid cryptographic hashing algorithm in %sbug02104-002.php on line 2

ValueError: hash_hmac(): Argument #1 ($algo) must be a valid cryptographic hashing algorithm in %sbug02104-002.php on line 2

Call Stack:
%w%f %w%d   1. {main}() %sbug02104-002.php:0
%w%f %w%d   2. hash_hmac($algo = 'foo', $data = 'bar', $key = '[Sensitive Parameter]') %sbug02104-002.php:2
