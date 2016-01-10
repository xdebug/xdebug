--TEST--
Test for bug #1245: xdebug_dump_superglobals dumps *uninitialized* with PHP 7
--INI--
xdebug.default_enable=1
xdebug.collect_params=4
html_errors=0
xdebug.dump.GET=*
xdebug.dump.POST=*
--GET--
getFoo=bar
--POST--
postBar=baz
--FILE--
<?php
trigger_error("TEST");
?>
--EXPECTF--
Notice: TEST in %sbug01245.php on line 2

Call Stack:
%w%f %w%d   1. {main}() %sbug01245.php:0
%w%f %w%d   2. trigger_error('TEST') %sbug01245.php:2

Dump $_GET
   $_GET['getFoo'] = 'bar'
Dump $_POST
   $_POST['postBar'] = 'baz'
