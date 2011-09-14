--TEST--
Test for nested static method calls
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=3
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.show_mem_delta=0
xdebug.trace_format=0
--FILE--
<?php
$tf = xdebug_start_trace('/tmp/'. uniqid('xdt', TRUE));
class DBHelper
{
  static function quote($s) {
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
%w%f %w%d     -> DBHelper::quote('test\'s') /%s/test9b.php:18
%w%f %w%d       -> addslashes('test\'s') /%s/test9b.php:6
%w%f %w%d     -> DBHelper::quote('test\'s') /%s/test9b.php:18
%w%f %w%d       -> addslashes('test\'s') /%s/test9b.php:6
%w%f %w%d     -> DB->query('insert blah \'test\\\'stest\\\'s\' blah') /%s/test9b.php:18
%w%f %w%d     -> DB->query('insert blah \' blah') /%s/test9b.php:19
%w%f %w%d     -> file_get_contents('/tmp/%s') /%s/test9b.php:20
