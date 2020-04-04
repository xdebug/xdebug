--TEST--
Test for bug #1215: SIGSEGV if xdebug.output_dir directory does not exist
--INI--
xdebug.default_enable=1
xdebug.auto_trace=1
xdebug.output_dir=/tmp/jumberwocky
--FILE--
<?php
echo strlen("hi!"), "\n";
?>
--EXPECT--
3
