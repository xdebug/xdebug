--TEST--
Test for bug #2222: a destructor can throw while its cached trace is evicted
--INI--
xdebug.mode=develop
--FILE--
<?php
class ReentrantGuard
{
	public function __destruct()
	{
		try {
			throw new RuntimeException('during cache eviction');
		} catch (RuntimeException $exception) {
			echo 'destructor trace available: ', xdebug_get_function_stack(['from_exception' => $exception]) !== [] ? 'yes' : 'no', "\n";
		}
	}
}

function makeException(bool $withGuard): Throwable
{
	if ($withGuard) {
		$guard = new ReentrantGuard();
	}
	try {
		throw new RuntimeException('retained');
	} catch (RuntimeException $exception) {
		return $exception;
	}
}

$exceptions = [];
for ($i = 0; $i < 9; $i++) {
	$exceptions[] = makeException($i === 0);
}
echo 'newest trace available: ', xdebug_get_function_stack(['from_exception' => $exceptions[8]]) !== [] ? 'yes' : 'no', "\n";
?>
--EXPECT--
destructor trace available: yes
newest trace available: yes
