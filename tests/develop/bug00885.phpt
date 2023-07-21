--TEST--
Test for bug #885: missing validation point returned by strchr in xdebug_error_cb
--INI--
xdebug.mode=develop
--FILE--
<?php
throw new Exception("long message ".str_repeat('.', 10240));
?>
--EXPECTF--
Fatal error: Uncaught%sException%slong message %s in %sbug00885.php on line 2

Exception: long message %s in %sbug00885.php on line 2

Call Stack:
%w%f %w%d   1. {main}() %sbug00885.php:0
