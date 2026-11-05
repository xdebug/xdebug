--TEST--
xdebug_add_source_map_directory()
--SKIPIF--
<?php
require __DIR__ . '/../../../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require __DIR__ . '/../../dbgp/dbgpclient.php';

$filename = dirname(__FILE__) . '/add-source-map-directory-001.inc';

$xdebugLogFileName = sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'add-source-map-directory-001';
@unlink( $xdebugLogFileName );

$commands = array(
	'feature_set -n breakpoint_details -v 1',
	'feature_set -n resolved_breakpoints -v 1',
	"breakpoint_set -t line -f {$filename} -n 4",
	'breakpoint_list',
	'run',
	"breakpoint_set -t line -f /var/www/projects/xdebug-test/fake-local-file.php -n 6",
	'breakpoint_list',
	'step_over',
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
[%d] [Step Debug] <- step_over -i 8
%A
[%d] [Step Debug] DEBUG: Breakpoint %d0002 (type: line).
[%d] [Path Mapping] INFO: Mapping (to replace) local location /var/www/projects/xdebug-test/fake-local-file.php:6
[%d] [Path Mapping] INFO: Mapped location /var/www/projects/xdebug-test/fake-local-file.php:6 to %sadd-source-map-directory-001.inc:6
%A
[%d] [Step Debug] -> <response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_over" transaction_id="8" status="break" reason="ok"><xdebug:message filename="file://%sadd-source-map-directory-001.inc" lineno="5"></xdebug:message></response>

[%d] [Step Debug] <- breakpoint_list -i 9
[%d] [Path Mapping] INFO: Mapping brkinfo local location %sadd-source-map-directory-001.inc:4
[%d] [Path Mapping] INFO: Couldn't map location %sadd-source-map-directory-001.inc:4
[%d] [Path Mapping] INFO: Mapping brkinfo local location %sadd-source-map-directory-001.inc:6
[%d] [Path Mapping] INFO: Mapped location %sadd-source-map-directory-001.inc:6 to /var/www/projects/xdebug-test/fake-local-file.php:6
[%d] [Step Debug] -> <response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_list" transaction_id="9"><breakpoint type="line" resolved="resolved" filename="file:///%sadd-source-map-directory-001.inc" lineno="4" state="enabled" hit_count="1" hit_value="0" id="%d0001"></breakpoint><breakpoint type="line" resolved="unresolved" filename="file:///var/www/projects/xdebug-test/fake-local-file.php" lineno="6" state="enabled" hit_count="0" hit_value="0" id="%d0002"></breakpoint></response>
%A
