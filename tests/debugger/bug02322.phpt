--TEST--
Test for bug #2322: Xdebug tries to open debugging connection in destructors during shutdown
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--INI--
xdebug.mode=debug
xdebug.start_with_request=yes
xdebug.log=
--FILE--
<?php
class Testing
{
	function __destruct() {
		echo "Destruct\n";
	}
}

$t = array();
for ($i=0; $i<5; $i++) {
	$t[] = new Testing();
}
?>
--EXPECTF--
Xdebug: [Step Debug] Could not connect to debugging client.%s
Destruct
Destruct
Destruct
Destruct
Destruct
