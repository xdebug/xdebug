--TEST--
Test for bug #1215: SIGSEGV if xdebug.output_dir directory does not exist
--INI--
xdebug.mode=trace
xdebug.start_with_request=default
xdebug.output_dir=/tmp/jumberwocky
--FILE--
<?php
echo strlen("hi!"), "\n";
?>
--EXPECT--
3
