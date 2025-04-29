--TEST--
DBGp: detach
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/dbgp-detach.inc';

$commands = array(
	'detach -- This is the reason for detaching.',
);

dbgpRunFile( $filename, $commands );

echo file_get_contents(getTmpFile('php-stdout.txt'));
?>
--EXPECTF--
%A
-> detach -i %d -- This is the reason for detaching.
%A
%sDebug client detached: This is the reason for detaching.
%A
Detached => This is the reason for detaching.
%A
