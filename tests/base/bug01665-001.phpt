--TEST--
Test for bug #1665: Segfault when overriding a function object parameter + xdebug.collect_params=1
--INI--
xdebug.default_enable=1
xdebug.collect_params=1
--FILE--
<?php

function query($var, $replaceWith) {
	try {
		// Show developer the $var/$replaceWith values or array indexes so they know which case failed (if phpt is run directly)
		if (preg_match('~\\.phpt$~', __FILE__)) {
			echo 'Testing gettype($var) = ' . gettype($var) . ', gettype($replaceWith) = ' . gettype($replaceWith) . PHP_EOL;
		}
		$var = $replaceWith;
		throw new LogicException('I am broken on purpose (to trigger Xdebug stack trace collector)');
	} catch (Exception $ex) {
	}
}
$replacedList = array(
	"What about a string?",
	"What about a longer string?",
	fopen("php://output", 'r'),
	0,
	3.1415,
	array("Or", "an", "array", "of", 6, "elements?"),
	array(new stdClass(), "an array with an object?"),
	false,
	true,
	null,
	new stdClass(),
	static function (): string { return "I'm a callable!"; });

$replacingWithList = array(
	// The string is longer than the original $var (for 1st case) and shorter (for 2nd case)
	"Country roooooooooooads",
	fopen("php://input", 'r'),
	42,
	2.71828182,
	array(7, "elements", "are", "more", "than", "original", 6.0),
	array("Array ending with an object", new stdClass()),
	false,
	true,
	null,
	new stdClass(),
	static function (): string { return "I'm a replacing callable!"; });

foreach ($replacedList as $replaced) {
	foreach ($replacingWithList as $replacingWith) {
		query($replaced, $replacingWith);
	}
}
echo 'No segfault';
?>
--EXPECTF--
No segfault
