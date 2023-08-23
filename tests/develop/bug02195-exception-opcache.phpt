--TEST--
Test for bug #2195: xdebug_get_function_stack(['from_exception']) (opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('opcache');
?>
--INI--
xdebug.mode=develop
xdebug.auto_profile=0
xdebug.var_display_max_depth=4
--FILE--
<?php
class Handlers
{
	function __construct(private string $title, private float $PIE) {}

	static function exceptionHandler($exception)
	{
		var_dump( xdebug_get_function_stack( [ 'local_vars' => true ] ), xdebug_get_function_stack( [ 'from_exception' => $exception ] ) );
	}
}

class Elephpant
{
	function __construct(private string $title, private string $PIE) {}
}

class Error_Class
{
	public static function newError($errno = false)
	{
		$elephpant = new Elephpant("Bluey", M_PI);
		$randoVar = 42;

		throw new Exception();
	}

}

class Error_Entry
{
	public function __construct($base, $errno)
	{
		$return = Error_Class::newError(true);
	}
}

set_exception_handler(['Handlers', 'exceptionHandler']);
$e = new Error_Entry(1, 2);

?>
--EXPECTF--
%sbug02195-exception-opcache.php:8:
array(1) {
  [0] =>
  array(9) {
    'time' =>
    double(%f)
    'memory' =>
    int(%d)
    'function' =>
    string(16) "exceptionHandler"
    'type' =>
    string(6) "static"
    'class' =>
    string(8) "Handlers"
    'file' =>
    string(%d) "%sbug02195-exception-opcache.php"
    'line' =>
    int(0)
    'params' =>
    array(1) {
      'exception' =>
      string(%d) "class Exception { %s
    }
    'variables' =>
    array(1) {
      'exception' =>
      class Exception#3 (7) {
        protected $message =>
        string(0) ""
        private %S$string =>
        string(0) ""
        protected $code =>
        int(0)
        protected %S$file =>
        string(%d) "%sbug02195-exception-opcache.php"
        protected %S$line =>
        int(24)
        private array $trace =>
        array(2) {
          ...
        }
        private ?Throwable $previous =>
        NULL
      }
    }
  }
}
%sbug02195-exception-opcache.php:8:
array(3) {
  [0] =>
  array(7) {
    'time' =>
    double(%f)
    'memory' =>
    int(%d)
    'function' =>
    string(6) "{main}"
    'file' =>
    string(%d) "%sbug02195-exception-opcache.php"
    'line' =>
    int(0)
    'params' =>
    array(0) {
    }
    'variables' =>
    array(1) {
      'e' =>
      NULL
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
    string(%d) "%sbug02195-exception-opcache.php"
    'line' =>
    int(38)
    'params' =>
    array(2) {
      'base' =>
      int(1)
      'errno' =>
      int(2)
    }
    'variables' =>
    array(3) {
      'base' =>
      int(1)
      'errno' =>
      int(2)
      'return' =>
      NULL
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
    string(%d) "%sbug02195-exception-opcache.php"
    'line' =>
    int(33)
    'params' =>
    array(1) {
      'errno' =>
      bool(true)
    }
    'variables' =>
    array(2) {
      'errno' =>
      bool(true)
      'elephpant' =>
      class Elephpant#%d (2) {
        private string $title =>
        string(5) "Bluey"
        private string $PIE =>
        string(%d) "3.14159265%d"
      }
    }
  }
}
