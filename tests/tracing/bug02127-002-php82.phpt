--TEST--
Test for bug #2127: Tracing does not handle NUL char in anonymous closure scope (>= PHP 8.2)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.2');
?>
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.collect_return=1
xdebug.collect_assignments=1
xdebug.trace_format=1
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
Version: %s
File format: 4
TRACE START [%d-%d-%d %d:%d:%d.%d]
2		A						%scapture-trace.inc	17	$tf = '%s'
2	1	1	%f	%d
2	7	0	%f	%d	Test->__construct	1		%sbug02127-002-php82.php	18	0
3	8	0	%f	%d	Exception->__construct	0		%sbug02127-002-php82.php	13	0
3	8	1	%f	%d
3	8	R			NULL
3	9	0	%f	%d	Closure::bind	0		%sbug02127-002-php82.php	10	3	class Closure { public $parameter = ['$a' => '<required>', '$b' => '<required>'] }	NULL	class Exception@anonymous { protected $message = ''; private string ${Exception}string = ''; protected $code = 0; protected string $file = '%sbug02127-002-php82.php'; protected int $line = 13; private array ${Exception}trace = [0 => [...]]; private ?Throwable ${Exception}previous = NULL }
3	9	1	%f	%d
3	9	R			class Closure { public $parameter = ['$a' => '<required>', '$b' => '<required>'] }
2		A						%sbug02127-002-php82.php	10	$this->configureException = class Closure { public $parameter = ['$a' => '<required>', '$b' => '<required>'] }
2	7	1	%f	%d
1		A						%sbug02127-002-php82.php	18	$t = class Test { private $configureException = class Closure { public $parameter = [...] } }
2	10	0	%f	%d	xdebug_stop_trace	0		%sbug02127-002-php82.php	20	0
			%f	%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
