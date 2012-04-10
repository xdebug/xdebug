--TEST--
Test for bug #241: Crash in xdebug_get_function_stack()
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
xdebug.collect_vars=1
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.var_display_max_depth=3
xdebug.var_display_max_children=2
xdebug.overload_var_dump=0
--FILE--
<?php
function error_handler($errno, $string, $file, $line, $context)
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

$tmp = explode('/', trim($_SERVER['FOO'], '/'));
echo "The End\n";
?>
--EXPECTF--
array(5) {
  [0]=>
  array(4) {
    ["function"]=>
    string(6) "{main}"
    ["file"]=>
    string(%d) "%sbug00241.php"
    ["line"]=>
    int(0)
    ["params"]=>
    array(0) {
    }
  }
  [1]=>
  array(4) {
    ["function"]=>
    string(13) "error_handler"
    ["file"]=>
    string(%d) "%sbug00241.php"
    ["line"]=>
    int(32)
    ["params"]=>
    array(5) {
      ["errno"]=>
      string(1) "8"
      ["string"]=>
      string(2%d) "'Undefined index:%sFOO'"
      ["file"]=>
      string(%d) "'%sbug00241.php'"
      ["line"]=>
      string(2) "32"
      ["context"]=>
      string(%d) "array (%s)"
    }
  }
  [2]=>
  array(6) {
    ["function"]=>
    string(8) "newError"
    ["type"]=>
    string(6) "static"
    ["class"]=>
    string(11) "Error_Class"
    ["file"]=>
    string(%d) "%sbug00241.php"
    ["line"]=>
    int(4)
    ["params"]=>
    array(1) {
      ["errno"]=>
      string(%d) "%s
    }
  }
  [3]=>
  array(6) {
    ["function"]=>
    string(11) "__construct"
    ["type"]=>
    string(7) "dynamic"
    ["class"]=>
    string(11) "Error_Entry"
    ["file"]=>
    string(%d) "%sbug00241.php"
    ["line"]=>
    int(11)
    ["params"]=>
    array(2) {
      ["base"]=>
      string(5) "FALSE"
      ["errno"]=>
      string(5) "FALSE"
    }
  }
  [4]=>
  array(6) {
    ["function"]=>
    string(5) "getBT"
    ["type"]=>
    string(6) "static"
    ["class"]=>
    string(11) "Error_Class"
    ["file"]=>
    string(%d) "%sbug00241.php"
    ["line"]=>
    int(26)
    ["params"]=>
    array(0) {
    }
  }
}
The End
