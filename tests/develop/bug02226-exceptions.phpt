--TEST--
Test for bug #2195: xdebug_get_function_stack(['from_exception']) (!opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('!opcache');
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
		echo "In exceptionHandler:\n";
		$stack = xdebug_get_function_stack( [ 'from_exception' => $exception ] );
		echo "\tThere are ", count( $stack ), " stack frames\n";
	}
}

class Error_Entry
{
	public function __construct($base, $errno)
	{
		try {
			throw new Exception("Numbers: {$base}/{$errno}\n");
		} catch (Throwable $t) {
		}
		throw new Exception("Second: {$base}/{$errno}\n");
	}
}

set_exception_handler(['Handlers', 'exceptionHandler']);
$e = new Error_Entry(1, 2);

?>
--EXPECTF--
In exceptionHandler:
	There are 2 stack frames
