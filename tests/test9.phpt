--TEST--
Test for nested static method calls
--INI--
xdebug.enable=1
xdebug.auto_trace=0
--FILE--
<?php
xdebug_start_trace();
class DBHelper
{
  function quote($s) {
    return addslashes ($s);
  }
}

class DB
{
  function query($s) {
  }
}

$db = new DB;

$db->query("insert blah '".DBHelper::quote("test's").DBHelper::quote("test's")."' blah");
$db->query("insert blah ' blah");
xdebug_dump_function_trace();
?>
--EXPECTF--
Function trace:
    %f      %d     -> dbhelper::quote('test\'s') /%s/test9.php:18
    %f      %d       -> addslashes('test\'s') /%s/test9.php:6
    %f      %d     -> dbhelper::quote('test\'s') /%s/test9.php:18
    %f      %d       -> addslashes('test\'s') /%s/test9.php:6
    %f      %d     -> db->query('insert blah \'test\\\'stest\\\'s\' blah') /%s/test9.php:18
    %f      %d     -> db->query('insert blah \' blah') /%s/test9.php:19
