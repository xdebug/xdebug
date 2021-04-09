--TEST--
Starting Profiler: default, no environment
--INI--
xdebug.mode=profile
xdebug.start_with_request=default
xdebug.collect_return=0
xdebug.collect_assignments=0
--FILE--
<?php
$fileName = xdebug_get_profiler_filename();

$fp = preg_match( '@gz$@', $fileName ) ? gzopen( $fileName, 'r' ) : fopen( $fileName, 'r' );
echo stream_get_contents( $fp );

@unlink($fileName);
?>
--EXPECTF--
version: 1
creator: xdebug %d.%s (PHP %s)
cmd: %sstart_with_request_default_no_env.php
part: 1
positions: line

events: Time_(10ns) Memory_(bytes)
