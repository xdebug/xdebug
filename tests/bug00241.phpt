--TEST--
Test for bug #241: Crash in xdebug_get_function_stack().
--SKIPIF--
<?php if (!extension_loaded("xdebug")) print "skip"; ?>
<?php if(version_compare(zend_version(), "2.0.0-dev", '<')) echo "skip Zend Engine 2 needed\n"; ?>
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
xdebug.collect_vars=1
xdebug.auto_profile=0
xdebug.profiler_enable=0
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
    int(0)
    ["params"]=>
    array(5) {
      ["errno"]=>
      string(1) "8"
      ["string"]=>
      string(23) "'Undefined index:  FOO'"
      ["file"]=>
      string(%d) "'%sbug00241.php'"
      ["line"]=>
      string(2) "32"
      ["context"]=>
      string(98) "array ('GLOBALS' => ..., '_ENV' => array ('SSH_AGENT_PID' => '3466', 'TERM' => 'xterm', ...), ...)"
    }
  }
  [2]=>
  array(5) {
    ["function"]=>
    string(8) "newError"
    ["class"]=>
    string(11) "Error_Class"
    ["file"]=>
    string(%d) "%sbug00241.php"
    ["line"]=>
    int(4)
    ["params"]=>
    array(1) {
      ["errno"]=>
      string(4) "NULL"
    }
  }
  [3]=>
  array(5) {
    ["function"]=>
    string(11) "__construct"
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
  array(5) {
    ["function"]=>
    string(5) "getBT"
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
