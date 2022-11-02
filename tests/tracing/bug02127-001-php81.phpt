--TEST--
Test for bug #2127: Tracing does not handle NUL char in anonymous closure scope (< PHP 8.2)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP < 8.2');
?>
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.collect_return=1
xdebug.collect_assignments=1
xdebug.trace_format=0
--FILE--
<?php
require_once 'capture-trace.inc';

class Test
{
	private $configureException;

	function __construct()
	{
		$this->configureException = \Closure::bind(
			static function($a, $b) { return $a * $b; },
			null,
			new class() extends \Exception {}
		);
	}
}

$t = new Test;

xdebug_stop_trace();
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w               => $tf = '%s' %s
%w%f %w%d     -> Test->__construct() %sbug02127-001-php81.php:18
%w%f %w%d       -> Exception->__construct() %sbug02127-001-php81.php:13
%w%f %w%d        >=> NULL
%w%f %w%d       -> Closure::bind($closure = class Closure { virtual $closure = "Test::{closure}", public $parameter = ['$a' => '<required>', '$b' => '<required>'] }, $newThis = NULL, $newScope = class Exception@anonymous { protected $message = ''; private %S${Exception}string = ''; protected $code = 0; protected %S$file = '%sbug02127-001-php81.php'; protected %S$line = 13; private array ${Exception}trace = [0 => [...]]; private ?Throwable ${Exception}previous = NULL }) %sbug02127-001-php81.php:13
%w%f %w%d        >=> class Closure { virtual $closure = "Exception@anonymous::{closure}", public $parameter = ['$a' => '<required>', '$b' => '<required>'] }
                             => $this->configureException = class Closure { virtual $closure = "Exception@anonymous::{closure}", public $parameter = ['$a' => '<required>', '$b' => '<required>'] } %sbug02127-001-php81.php:10
                           => $t = class Test { private $configureException = class Closure { virtual $closure = "Exception@anonymous::{closure}", public $parameter = [...] } } %sbug02127-001-php81.php:18
%w%f %w%d     -> xdebug_stop_trace() %sbug02127-001-php81.php:20
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
