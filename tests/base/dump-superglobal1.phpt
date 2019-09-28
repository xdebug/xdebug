--TEST--
Test for dumping of super globals
--INI--
xdebug.default_enable=1
xdebug.dump_globals=1
xdebug.dump_once=0
xdebug.dump.SERVER=argc
xdebug.dump.GET=
xdebug.collect_params=4
--FILE--
<?php
trigger_error('foo');
echo "-------------\n";

ini_set('xdebug.dump.SERVER', '');
trigger_error('foo');
echo "-------------\n";

ini_set('xdebug.dump.SERVER', 'argc,argv');
trigger_error('foo');
?>
--EXPECTF--
Notice: foo in %sdump-superglobal1.php on line 2

Call Stack:
%w%f %w%d   1. {main}() %sdump-superglobal1.php:0
%w%f %w%d   2. trigger_error('foo') %sdump-superglobal1.php:2

Dump $_SERVER
   $_SERVER['argc'] = 1
-------------

Notice: foo in %sdump-superglobal1.php on line 6

Call Stack:
%w%f %w%d   1. {main}() %sdump-superglobal1.php:0
%w%f %w%d   2. trigger_error('foo') %sdump-superglobal1.php:6

-------------

Notice: foo in %sdump-superglobal1.php on line 10

Call Stack:
%w%f %w%d   1. {main}() %sdump-superglobal1.php:0
%w%f %w%d   2. trigger_error('foo') %sdump-superglobal1.php:10

Dump $_SERVER
   $_SERVER['argc'] = 1
   $_SERVER['argv'] = array (0 => '%sdump-superglobal1.php')
