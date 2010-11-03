--TEST--
Test for bug #566: Xdebug crashes when using conditional breakpoints [1]
--INI--
xdebug.collect_params=4
xdebug.collect_return=1
xdebug.collect_assignments=0
--FILE--
<?php
$tf = xdebug_start_trace('%s'. uniqid('xdt', TRUE));
function loadMod( $module )
{
	strlen( $module );
	$module .= 's';
	strlen( $module );
}

function loadFoo( $test )
{
	strlen( $test );
	$test .= 's';
	strlen( $test );
}

loadFoo( 'view' );
loadMod( 'test' );
loadMod( 'view' );

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d]
%w%f %w%d     -> loadFoo($test = 'view') %sbug00566-1.php:17
%w%f %w%d       -> strlen('view') %sbug00566-1.php:12
                             >=> 4
%w%f %w%d       -> strlen('views') %sbug00566-1.php:14
                             >=> 5
                           >=> NULL
%w%f %w%d     -> loadMod($module = 'test') %sbug00566-1.php:18
%w%f %w%d       -> strlen('test') %sbug00566-1.php:5
                             >=> 4
%w%f %w%d       -> strlen('tests') %sbug00566-1.php:7
                             >=> 5
                           >=> NULL
%w%f %w%d     -> loadMod($module = 'view') %sbug00566-1.php:19
%w%f %w%d       -> strlen('view') %sbug00566-1.php:5
                             >=> 4
%w%f %w%d       -> strlen('views') %sbug00566-1.php:7
                             >=> 5
                           >=> NULL
%w%f %w%d     -> xdebug_stop_trace() %sbug00566-1.php:21
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d]
