--TEST--
Test for bug #2222: discarded exceptions release captured objects at scope exit
--INI--
xdebug.mode=develop
--FILE--
<?php
class ScopedGuard
{
	public function __destruct()
	{
		echo "guard destroyed\n";
	}
}

function work(): void
{
	$guard = new ScopedGuard();
	try {
		throw new RuntimeException('handled');
	} catch (RuntimeException) {
	}
}

work();
echo "scope returned\n";
?>
--EXPECT--
guard destroyed
scope returned
