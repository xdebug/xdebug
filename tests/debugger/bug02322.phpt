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
xdebug.log={TMP}/{RUNID}{TEST_PHP_WORKER}bug02322.txt
--FILE--
<?php
class Testing
{
	private $i;

	function __construct($i) {
		$this->i = $i;
	}

	function __destruct() {
		echo "Destruct\n";
		if ($this->i == 4) {
			echo file_get_contents(sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'bug02322.txt' );
			@unlink (sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'bug02322.txt' );
		}
	}
}

$t = array();
for ($i=0; $i<5; $i++) {
	$t[] = new Testing($i);
}

?>
--EXPECTF--
Destruct
Destruct
Destruct
Destruct
Destruct
%A[Step Debug] %s Tried: localhost:9003 (through xdebug.client_host/xdebug.client_port).
