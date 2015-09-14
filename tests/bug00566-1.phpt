--TEST--
Test for bug #566: Xdebug crashes when using conditional breakpoints (1)
--INI--
xdebug.collect_params=4
xdebug.collect_return=1
xdebug.collect_assignments=0
xdebug.trace_format=0
xdebug.auto_trace=0
--FILE--
<?php
$tf = xdebug_start_trace('%s'. uniqid('xdt', TRUE));
function loadMod( $module )
{
	strrev( $module );
	$module .= 's';
	strrev( $module );
}

function loadFoo( $test )
{
	strrev( $test );
	$test .= 's';
	strrev( $test );
}

$a = loadFoo( 'view' );
$a = loadMod( 'test' );
$a = loadMod( 'view' );

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d]
%w%f %w%d     -> loadFoo($test = 'view') %sbug00566-1.php:17
%w%f %w%d       -> strrev('view') %sbug00566-1.php:12
%w%f %w%d        >=> 'weiv'
%w%f %w%d       -> strrev('views') %sbug00566-1.php:14
%w%f %w%d        >=> 'sweiv'
%w%f %w%d      >=> NULL
%w%f %w%d     -> loadMod($module = 'test') %sbug00566-1.php:18
%w%f %w%d       -> strrev('test') %sbug00566-1.php:5
%w%f %w%d        >=> 'tset'
%w%f %w%d       -> strrev('tests') %sbug00566-1.php:7
%w%f %w%d        >=> 'stset'
%w%f %w%d      >=> NULL
%w%f %w%d     -> loadMod($module = 'view') %sbug00566-1.php:19
%w%f %w%d       -> strrev('view') %sbug00566-1.php:5
%w%f %w%d        >=> 'weiv'
%w%f %w%d       -> strrev('views') %sbug00566-1.php:7
%w%f %w%d        >=> 'sweiv'
%w%f %w%d      >=> NULL
%w%f %w%d     -> xdebug_stop_trace() %sbug00566-1.php:21
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d]
