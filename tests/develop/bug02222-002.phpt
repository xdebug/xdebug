--TEST--
Test for bug #2222: live exceptions preserve their captured local values
--INI--
xdebug.mode=develop
--FILE--
<?php
function work(): array
{
	$guard = new stdClass();
	$reference = WeakReference::create($guard);
	try {
		throw new RuntimeException('retained');
	} catch (RuntimeException $exception) {
		return [$exception, $reference];
	}
}

[$exception, $reference] = work();
echo 'guard alive: ', $reference->get() !== null ? 'yes' : 'no', "\n";
$trace = xdebug_get_function_stack(['from_exception' => $exception]);
$frame = end($trace);
echo 'original local value: ', $frame['variables']['guard'] === $reference->get() ? 'yes' : 'no', "\n";
unset($trace, $frame, $exception);
echo 'guard released: ', $reference->get() === null ? 'yes' : 'no', "\n";
?>
--EXPECT--
guard alive: yes
original local value: yes
guard released: yes
