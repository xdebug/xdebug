<?php
xdebug_start_trace();
class DBHelper
{
  function quote($s) {
    return str_replace("'", "''", $s);
  }
}

function blaat($a) {
}

blaat("insert blah '".DBHelper::quote("test's")."' blah");
xdebug_dump_function_trace();
?>
