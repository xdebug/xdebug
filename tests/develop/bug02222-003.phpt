--TEST--
Test for bug #2222: trace values referring to their exception can be collected
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.3');
?>
--INI--
xdebug.mode=develop
--FILE--
<?php
function work(): WeakReference
{
	$guard = new stdClass();
	$reference = WeakReference::create($guard);
	$exception = new RuntimeException('captured in its own trace');
	try {
		throw $exception;
	} catch (RuntimeException) {
	}
	return $reference;
}

$reference = work();
echo 'guard alive before GC: ', $reference->get() !== null ? 'yes' : 'no', "\n";
gc_collect_cycles();
echo 'guard released after GC: ', $reference->get() === null ? 'yes' : 'no', "\n";
?>
--EXPECT--
guard alive before GC: yes
guard released after GC: yes
