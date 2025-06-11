--TEST--
Test for bug #2352: Crash with storing exception traces and __invoke (= PHP 8.1)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.1, < 8.2');
?>
--INI--
xdebug.mode=develop
--FILE--
<?php
class RedisProxy {
	function __construct(public Closure $c)
	{
	}

	function __call($method, $args)
	{
		$this->c->__invoke($args);
	}
}

$c = function(array $args)
{
	throw new Exception();
};

$rp = new RedisProxy($c);
$rp->isConnected();
?>
--EXPECTF--
Fatal error: Uncaught Exception in %sbug02352-php81.php on line 15

Exception:  in %sbug02352-php81.php on line 15

Call Stack:
%w%f %w%d   1. {main}() %sbug02352-php81.php:0
%w%f %w%d   2. RedisProxy->__call($method = 'isConnected', $args = []) %sbug02352-php81.php:19
%w%f %w%d   3. Closure->__invoke(%s = []) %sbug02352-php81.php:9
%w%f %w%d   4. {closure:%sbug02352-php81.php:13-16}($args = []) %sbug02352-php81.php:9
