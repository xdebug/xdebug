--TEST--
Test for bug #565: xdebug.show_local_vars setting does not work with php 5.3
--INI--
xdebug.mode=develop
xdebug.show_local_vars=1
xdebug.dump.GET=
xdebug.dump.SERVER=
--FILE--
<?php
function func(){
	$a="hoge";
	throw new Exception($a);
}

func();
?>
--EXPECTF--
Fatal error: Uncaught%sException%sin %sbug00565.php on line 4

Exception:%sin %sbug00565.php on line 4

Call Stack:
%w%f %w%d   1. {main}() %sbug00565.php:0
%w%f %w%d   2. func() %sbug00565.php:7


Variables in local scope (#2):
  $a = 'hoge'
