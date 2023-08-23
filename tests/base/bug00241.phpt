--TEST--
Test for bug #241: Crash in xdebug_get_function_stack()
--INI--
xdebug.mode=develop
xdebug.auto_profile=0
xdebug.var_display_max_depth=3
xdebug.var_display_max_children=4
--FILE--
<?php
function error_handler($errno, $string, $file, $line)
{
	$entry = Error_Class::newError();
}

class Error_Class
{
	public static function newError($errno = false)
	{
		return new Error_Entry(false, $errno);
	}

	public static function getBT()
	{
		$tmp = xdebug_get_function_stack();
		var_dump($tmp);
	}

}

class Error_Entry
{
	public function __construct($base, $errno)
	{
		Error_Class::getBT();
	}
}

set_error_handler('error_handler');

$tmp = $_SERVER['FOO'];
echo "The End\n";
?>
--EXPECTF--
%sbug00241.php:17:
array(5) {
  [0] =>
  array(6) {
    'time' =>
    double(%f)
    'memory' =>
    int(%d)
    'function' =>
    string(6) "{main}"
    'file' =>
    string(%d) "%sbug00241.php"

    (more elements)...
  }
  [1] =>
  array(6) {
    'time' =>
    double(%f)
    'memory' =>
    int(%d)
    'function' =>
    string(13) "error_handler"
    'file' =>
    string(%d) "%sbug00241.php"

    (more elements)...
  }
  [2] =>
  array(8) {
    'time' =>
    double(%f)
    'memory' =>
    int(%d)
    'function' =>
    string(8) "newError"
    'type' =>
    string(6) "static"

    (more elements)...
  }
  [3] =>
  array(8) {
    'time' =>
    double(%f)
    'memory' =>
    int(%d)
    'function' =>
    string(11) "__construct"
    'type' =>
    string(7) "dynamic"

    (more elements)...
  }

  (more elements)...
}
The End
