<?php
class Foo
{
	public static function bar(string $input): string
	{
		return $input;
	}
}

$firstClassCallable = Foo::bar(...);
$closureFromCallable = Closure::fromCallable(['Foo', 'bar']);
$anonFunction = fn(string $string) => Foo::bar($string);

$firstClassCallable('testing');
?>
