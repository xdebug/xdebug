--TEST--
Filtered stack traces: path include [1]
--INI--
xdebug.mode=develop
--FILE--
<?php
$includeDir = realpath( __DIR__ . '/..' ); $s = DIRECTORY_SEPARATOR;

include "{$includeDir}/filter/stack/one.php";
include "{$includeDir}/filter/stack/two.php";
include "{$includeDir}/filter/stack/three.php";

$three = new \Stack\Three( new stdClass );
$two = new \Stack\Two( $three );
$one = new \Stack\One( $two );

xdebug_set_filter(XDEBUG_FILTER_STACK, XDEBUG_PATH_INCLUDE, [ "{$includeDir}{$s}filter{$s}stack{$s}one" ] );
$one->callObj( 'callObj', 'error', 'Error triggered!' );

xdebug_set_filter(XDEBUG_FILTER_STACK, XDEBUG_PATH_INCLUDE, [ "{$includeDir}{$s}filter{$s}stack{$s}one.php" ] );
$one->callObj( 'callObj', 'error', 'Error triggered!' );

xdebug_set_filter(XDEBUG_FILTER_STACK, XDEBUG_PATH_INCLUDE, [ "{$includeDir}{$s}filter{$s}stack{$s}one.p", "{$includeDir}{$s}filter{$s}stack{$s}two.php" ] );
$one->callObj( 'callObj', 'error', 'Error triggered!' );

xdebug_set_filter(XDEBUG_FILTER_STACK, XDEBUG_PATH_INCLUDE, [ "{$includeDir}{$s}filter{$s}stack{$s}t" ] );
$one->callObj( 'callObj', 'error', 'Error triggered!' );

xdebug_set_filter(XDEBUG_FILTER_STACK, XDEBUG_PATH_INCLUDE, [ 'x' ] );
$one->callObj( 'callObj', 'error', 'Error triggered!' );

xdebug_set_filter(XDEBUG_FILTER_STACK, XDEBUG_PATH_INCLUDE, [ "{$includeDir}" ] );
$one->callObj( 'callObj', 'error', 'Error triggered!' );

xdebug_set_filter(XDEBUG_FILTER_STACK, XDEBUG_PATH_INCLUDE, [ '' ] );
$one->callObj( 'callObj', 'error', 'Error triggered!' );
?>
--EXPECTF--
Warning: Error triggered! in %sthree.php on line 18

Call Stack:
%w%f %w%d   1. {main}() %sstack-filter-path-include-001.php:0
%w%f %w%d   3. Stack\Two->callObj($name = 'error', ...$arguments = variadic('Error triggered!')) %sone.php:13


Warning: Error triggered! in %sthree.php on line 18

Call Stack:
%w%f %w%d   1. {main}() %sstack-filter-path-include-001.php:0
%w%f %w%d   3. Stack\Two->callObj($name = 'error', ...$arguments = variadic('Error triggered!')) %sone.php:13


Warning: Error triggered! in %sthree.php on line 18

Call Stack:
%w%f %w%d   1. {main}() %sstack-filter-path-include-001.php:0
%w%f %w%d   3. Stack\Two->callObj($name = 'error', ...$arguments = variadic('Error triggered!')) %sone.php:13
%w%f %w%d   4. Stack\Three->error($value = 'Error triggered!') %stwo.php:13


Warning: Error triggered! in %sthree.php on line 18

Call Stack:
%w%f %w%d   1. {main}() %sstack-filter-path-include-001.php:0
%w%f %w%d   4. Stack\Three->error($value = 'Error triggered!') %stwo.php:13
%w%f %w%d   5. trigger_error($message = 'Error triggered!', $error_%s = 512) %sthree.php:18


Warning: Error triggered! in %sthree.php on line 18

Call Stack:
%w%f %w%d   1. {main}() %sstack-filter-path-include-001.php:0


Warning: Error triggered! in %sthree.php on line 18

Call Stack:
%w%f %w%d   1. {main}() %sstack-filter-path-include-001.php:0
%w%f %w%d   2. Stack\One->callObj($name = 'callObj', ...$arguments = variadic('error', 'Error triggered!')) %sstack-filter-path-include-001.php:28
%w%f %w%d   3. Stack\Two->callObj($name = 'error', ...$arguments = variadic('Error triggered!')) %sone.php:13
%w%f %w%d   4. Stack\Three->error($value = 'Error triggered!') %stwo.php:13
%w%f %w%d   5. trigger_error($message = 'Error triggered!', $error_%s = 512) %sthree.php:18


Warning: Error triggered! in %sthree.php on line 18

Call Stack:
%w%f %w%d   1. {main}() %sstack-filter-path-include-001.php:0
%w%f %w%d   2. Stack\One->callObj($name = 'callObj', ...$arguments = variadic('error', 'Error triggered!')) %sstack-filter-path-include-001.php:31
%w%f %w%d   3. Stack\Two->callObj($name = 'error', ...$arguments = variadic('Error triggered!')) %sone.php:13
%w%f %w%d   4. Stack\Three->error($value = 'Error triggered!') %stwo.php:13
%w%f %w%d   5. trigger_error($message = 'Error triggered!', $error_%s = 512) %sthree.php:18
