--TEST--
Test for bug #2354: The __invoke frame in call stacks don't have the argument name in the trace (< PHP 8.2)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP < 8.1');
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
%A
%w%f %w%d   %d. Closure->__invoke($args = []) %sbug02354.php:9
%A
