--TEST--
Test for bug #1562: Variables with xdebug_get_function_stack (!opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('!opcache');
?>
--INI--
xdebug.mode=develop
xdebug.auto_profile=0
xdebug.var_display_max_depth=4
serialize_precision=-1
--FILE--
<?php
class Elephpant
{
	function __construct(private string $title, private float $PIE) {}

	function __toString()
	{
		return "{$this->title} loves {$this->PIE}";
	}
}

class Error_Class
{
	public static function getBT()
	{
		$tmp = xdebug_get_function_stack( ['local_vars' => true ] );
		var_dump($tmp);

		echo $tmp[2]['variables']['errno'], "\n";
	}

	public static function newError($errno = false)
	{
		$elephpant = new Elephpant("Bluey", M_PI);
		$randoVar = 42;
		return self::getBT();
	}

}

class Error_Entry
{
	public function __construct($base, $errno)
	{
		$return = Error_Class::newError(true);
	}
}

$e = new Error_Entry(1, 2);
?>
--EXPECTF--
%sbug01562-noopcache.php:17:
array(4) {
  [0] =>
  array(6) {
    'time' =>
    double(%f)
    'memory' =>
    int(%d)
    'function' =>
    string(6) "{main}"
    'file' =>
    string(%d) "%sbug01562-noopcache.php"
    'line' =>
    int(0)
    'params' =>
    array(0) {
    }
  }
  [1] =>
  array(9) {
    'time' =>
    double(%f)
    'memory' =>
    int(%d)
    'function' =>
    string(11) "__construct"
    'type' =>
    string(7) "dynamic"
    'class' =>
    string(11) "Error_Entry"
    'file' =>
    string(%d) "%sbug01562-noopcache.php"
    'line' =>
    int(39)
    'params' =>
    array(2) {
      'base' =>
      string(1) "1"
      'errno' =>
      string(1) "2"
    }
    'variables' =>
    array(2) {
      'base' =>
      int(1)
      'errno' =>
      int(2)
    }
  }
  [2] =>
  array(9) {
    'time' =>
    double(%f)
    'memory' =>
    int(%d)
    'function' =>
    string(8) "newError"
    'type' =>
    string(6) "static"
    'class' =>
    string(11) "Error_Class"
    'file' =>
    string(%d) "%sbug01562-noopcache.php"
    'line' =>
    int(35)
    'params' =>
    array(1) {
      'errno' =>
      string(4) "TRUE"
    }
    'variables' =>
    array(1) {
      'errno' =>
      bool(true)
    }
  }
  [3] =>
  array(9) {
    'time' =>
    double(%f)
    'memory' =>
    int(%d)
    'function' =>
    string(5) "getBT"
    'type' =>
    string(6) "static"
    'class' =>
    string(11) "Error_Class"
    'file' =>
    string(%d) "%sbug01562-noopcache.php"
    'line' =>
    int(26)
    'params' =>
    array(0) {
    }
    'variables' =>
    array(0) {
    }
  }
}
1
