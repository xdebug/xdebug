--TEST--
Test for nested static method calls
--INI--
xdebug.mode=trace
xdebug.start_with_request=0
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.trace_format=0
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));
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
xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> DBHelper::quote($s = 'test\'s') %snested-static-method-calls.php:18
%w%f %w%d       -> addslashes($str%S = 'test\'s') %snested-static-method-calls.php:6
%w%f %w%d     -> DBHelper::quote($s = 'test\'s') %snested-static-method-calls.php:18
%w%f %w%d       -> addslashes($str%S = 'test\'s') %snested-static-method-calls.php:6
%w%f %w%d     -> DB->query($s = 'insert blah \'test\\\'stest\\\'s\' blah') %snested-static-method-calls.php:18
%w%f %w%d     -> DB->query($s = 'insert blah \' blah') %snested-static-method-calls.php:19
%w%f %w%d     -> xdebug_stop_trace() %snested-static-method-calls.php:20
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
