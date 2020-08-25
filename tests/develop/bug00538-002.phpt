--TEST--
Test for bug #538: Error in watches and call stack parameter with string containing '\\'
--INI--
xdebug.mode=develop
--FILE--
<?php
    function call($param1, $param2, $param3)
    {
        echo $param1, "\n";
        echo $param2, "\n";
        echo $param3, "\n";

		var_dump(xdebug_get_function_stack());
    }

    $test=getcwd();
	$value = 'candena\\a\nb';
	echo $value, "\n";
    call($test, $value, 'caneda \\\a \\\\b \\\\\c|');
?>
--EXPECTF--
candena\a\nb
%s
candena\a\nb
caneda \\a \\b \\\c|
%sbug00538-002.php:8:
array(2) {
  [0] =>
  array(4) {
    'function' =>
    string(6) "{main}"
    'file' =>
    string(%d) "%sbug00538-002.php"
    'line' =>
    int(0)
    'params' =>
    array(0) {
    }
  }
  [1] =>
  array(4) {
    'function' =>
    string(4) "call"
    'file' =>
    string(%d) "%sbug00538-002.php"
    'line' =>
    int(14)
    'params' =>
    array(3) {
      'param1' =>
      string(%d) "'%s'"
      'param2' =>
      string(16) "'candena\\a\\nb'"
      'param3' =>
      string(29) "'caneda \\\\a \\\\b \\\\\\c|'"
    }
  }
}
