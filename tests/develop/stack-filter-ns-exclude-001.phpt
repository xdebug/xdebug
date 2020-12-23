--TEST--
Filtered stack traces: namespace exclude [1]
--INI--
xdebug.mode=develop
--FILE--
<?php
$includeDir = realpath( __DIR__ . '/..' );

include "{$includeDir}/filter/stack/one.php";
include "{$includeDir}/filter/stack/two.php";
include "{$includeDir}/filter/stack/three.php";

$three = new \Stack\Three( new stdClass );
$two = new \Stack\Two( $three );
$one = new \Stack\One( $two );

xdebug_set_filter(XDEBUG_FILTER_STACK, XDEBUG_NAMESPACE_EXCLUDE, [ 'Stack\One' ] );
$one->callObj( 'callObj', 'error', 'Error triggered!' );

xdebug_set_filter(XDEBUG_FILTER_STACK, XDEBUG_NAMESPACE_EXCLUDE, [ '\Stack\One' ] );
$one->callObj( 'callObj', 'error', 'Error triggered!' );

xdebug_set_filter(XDEBUG_FILTER_STACK, XDEBUG_NAMESPACE_EXCLUDE, [ '\Stack\One', 'Stack\Two' ] );
$one->callObj( 'callObj', 'error', 'Error triggered!' );

xdebug_set_filter(XDEBUG_FILTER_STACK, XDEBUG_NAMESPACE_EXCLUDE, [ 'Stack\Two', 'Stack\Three' ] );
$one->callObj( 'callObj', 'error', 'Error triggered!' );

xdebug_set_filter(XDEBUG_FILTER_STACK, XDEBUG_NAMESPACE_EXCLUDE, [ '', 'Stack\Two' ] );
$one->callObj( 'callObj', 'error', 'Error triggered!' );
?>
--EXPECTF--
Warning: Error triggered! in %sthree.php on line 18

Call Stack:
%w%f %w%d   1. {main}() %sstack-filter-ns-exclude-001.php:0
%w%f %w%d   3. Stack\Two->callObj($name = 'error', ...$arguments = variadic('Error triggered!')) %sone.php:13
%w%f %w%d   4. Stack\Three->error($value = 'Error triggered!') %stwo.php:13
%w%f %w%d   5. trigger_error($message = 'Error triggered!', $error_%s = 512) %sthree.php:18


Warning: Error triggered! in %sthree.php on line 18

Call Stack:
%w%f %w%d   1. {main}() %sstack-filter-ns-exclude-001.php:0
%w%f %w%d   3. Stack\Two->callObj($name = 'error', ...$arguments = variadic('Error triggered!')) %sone.php:13
%w%f %w%d   4. Stack\Three->error($value = 'Error triggered!') %stwo.php:13
%w%f %w%d   5. trigger_error($message = 'Error triggered!', $error_%s = 512) %sthree.php:18


Warning: Error triggered! in %sthree.php on line 18

Call Stack:
%w%f %w%d   1. {main}() %sstack-filter-ns-exclude-001.php:0
%w%f %w%d   4. Stack\Three->error($value = 'Error triggered!') %stwo.php:13
%w%f %w%d   5. trigger_error($message = 'Error triggered!', $error_%s = 512) %sthree.php:18


Warning: Error triggered! in %sthree.php on line 18

Call Stack:
%w%f %w%d   1. {main}() %sstack-filter-ns-exclude-001.php:0
%w%f %w%d   2. Stack\One->callObj($name = 'callObj', ...$arguments = variadic('error', 'Error triggered!')) %sstack-filter-ns-exclude-001.php:22
%w%f %w%d   5. trigger_error($message = 'Error triggered!', $error_%s = 512) %sthree.php:18


Warning: Error triggered! in %sthree.php on line 18

Call Stack:
%w%f %w%d   1. {main}() %sstack-filter-ns-exclude-001.php:0
%w%f %w%d   2. Stack\One->callObj($name = 'callObj', ...$arguments = variadic('error', 'Error triggered!')) %sstack-filter-ns-exclude-001.php:25
%w%f %w%d   4. Stack\Three->error($value = 'Error triggered!') %stwo.php:13
