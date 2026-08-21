--TEST--
Test for bug #2352: Crash with storing exception traces and __invoke
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
Fatal error: Uncaught Exception in %sbug02352.php on line 15

Exception:  in %sbug02352.php on line 15

Call Stack:
%w%f %w%d   1. {main}() %sbug02352.php:0
%w%f %w%d   2. RedisProxy->__call($method = 'isConnected', $args = []) %sbug02352.php:19
%w%f %w%d   3. {closure:%sbug02352.php:13-16}($args = []) %sbug02352.php:9
