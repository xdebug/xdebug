--TEST--
Test for bug #1437: Add X-Xdebug-Profile-Filename header
--ENV--
XDEBUG_CONFIG=
--INI--
xdebug.mode=profile,develop
xdebug.start_with_request=default
xdebug.filename_format=
xdebug.profiler_output_name=XDEBUG-PROFILE.%p
--FILE--
<?php
header( 'Location: bar');
header( 'HTTP/1.0 404 Not Found' );
var_dump( xdebug_get_headers( ) );
@unlink( xdebug_get_profiler_filename() );
?>
--EXPECTF--
%sbug01437.php:4:
array(2) {
  [0] =>
  string(%d) "X-Xdebug-Profile-Filename: %sXDEBUG-PROFILE%s"
  [1] =>
  string(13) "Location: bar"
}
