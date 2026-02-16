--TEST--
DBGP: minimum path discovery of .xdebug map files
--SKIPIF--
<?php
require __DIR__ . '/../../../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require __DIR__ . '/../../dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/foo/bar/file.inc';

$xdebugLogFileName = sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'minimum-path.txt';
@unlink( $xdebugLogFileName );

$commands = array(
	'feature_set -n breakpoint_details -v 1',
	'step_into',
	"breakpoint_set -t line -f /var/www/minimum-path/fake-file.php -n 3",
	'breakpoint_list',
	'run',
	'detach',
);

dbgpRunFile(
	$filename, $commands,
	[
		'xdebug.mode' => 'debug', 'xdebug.start_with_request' => 'yes',
		'xdebug.log' => $xdebugLogFileName, 'xdebug.log_level' => 10,
		'xdebug.path_mapping' => 'yes',
	]
);

echo file_get_contents( $xdebugLogFileName );
@unlink( $xdebugLogFileName );
?>
--EXPECTF--
%A
[%d] [Path Mapping] INFO: Scanning for map files with pattern '%smap-minimum-path%e.xdebug%e*.map'
[%d] [Path Mapping] INFO: Reading mapping file '%smap-minimum-path%e.xdebug%eminimum.map'
[%d] [Path Mapping] INFO: Scanning for map files with pattern '%smap-minimum-path%efoo%e.xdebug%e*.map'
[%d] [Path Mapping] DEBUG: No map files found with pattern '%smap-minimum-path%efoo%e.xdebug%e*.map'
[%d] [Path Mapping] INFO: Scanning for map files with pattern '%sfoo%ebar%e.xdebug%e*.map'
[%d] [Path Mapping] DEBUG: No map files found with pattern '%sfoo%ebar%e.xdebug%e*.map'
[%d] [Path Mapping] DEBUG: Found 1 path mapping rules
%A
[%d] [Step Debug] <- breakpoint_set -i 3 -t line -f /var/www/minimum-path/fake-file.php -n 3
[%d] [Path Mapping] INFO: Mapping (to replace) local location /var/www/minimum-path/fake-file.php:3
[%d] [Path Mapping] INFO: Mapped location /var/www/minimum-path/fake-file.php:3 to %sfile.inc:3
%A
[%d] [Step Debug] <- breakpoint_list -i 4
[%d] [Path Mapping] INFO: Mapping brkinfo local location %sfile.inc:3
[%d] [Path Mapping] INFO: Mapped location %sfile.inc:3 to /var/www/minimum-path/fake-file.php:3
[%d] [Step Debug] -> <response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_list" transaction_id="4"><breakpoint type="line" filename="file:///var/www/minimum-path/fake-file.php" lineno="3" state="enabled" hit_count="0" hit_value="0" id="%s0001"></breakpoint></response>
%A
