--TEST--
Test for nested static method calls
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
xdebug.auto_profile=0
--FILE--
<?php
xdebug_start_trace($tf = tempnam('/tmp', 'xdt'));
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
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d]
    %f      %d     -> dbhelper::quote('test\'s') /%s/test9.php:18
    %f      %d       -> addslashes('test\'s') /%s/test9.php:6
    %f      %d     -> dbhelper::quote('test\'s') /%s/test9.php:18
    %f      %d       -> addslashes('test\'s') /%s/test9.php:6
    %f      %d     -> db->query('insert blah \'test\\\'stest\\\'s\' blah') /%s/test9.php:18
    %f      %d     -> db->query('insert blah \' blah') /%s/test9.php:19
    %f      %d     -> file_get_contents('/tmp/%s') /%s/test9.php:20
