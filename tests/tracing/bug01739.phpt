--TEST--
Test for bug #1739: Tracing footer not written
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.trace_options=0
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.dump_globals=0
xdebug.trace_format=0
xdebug.use_compression=0
--FILE--
<?php
$id = getenv( 'UNIQ_RUN_ID' ) ? getenv( 'UNIQ_RUN_ID' ) : '';
xdebug_start_trace( sys_get_temp_dir() . '/1739' . $id );
function foo() {
	echo "bar\n";
}

foo();
?>
--AFTER--
<?php
$id = getenv( 'UNIQ_RUN_ID' ) ? getenv( 'UNIQ_RUN_ID' ) : '';
echo file_get_contents( sys_get_temp_dir() . '/1739' . $id . '.xt' );
unlink( sys_get_temp_dir() . '/1739' . $id . '.xt' );
?>
--EXPECTF--
bar
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> foo() %sbug01739.php:%d
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
