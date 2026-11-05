--TEST--
Test for bug #2222: weak trace entries preserve the eight-slot eviction policy
--INI--
xdebug.mode=develop
--FILE--
<?php
function makeException(): Throwable
{
	try {
		throw new RuntimeException('retained');
	} catch (RuntimeException $exception) {
		return $exception;
	}
}

$exceptions = [];
for ($i = 0; $i < 9; $i++) {
	$exceptions[] = makeException();
}
echo 'oldest evicted: ', xdebug_get_function_stack(['from_exception' => $exceptions[0]]) === [] ? 'yes' : 'no', "\n";
echo 'newest available: ', xdebug_get_function_stack(['from_exception' => $exceptions[8]]) !== [] ? 'yes' : 'no', "\n";
?>
--EXPECT--
oldest evicted: yes
newest available: yes
