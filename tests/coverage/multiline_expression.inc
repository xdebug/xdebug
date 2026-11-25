<?php
class Box
{
	public static $flag = true;
	public static $instance;
	public $next;
	public $value = 1;

	function get()
	{
		return $this;
	}
}

function f($value)
{
	return $value;
}

function test($a, $b, $box, $boxes, $i, $s, $callable)
{
	f(
		$a
		|| $b
	);

	f(
		$a
		?: $b
	);

	f(
		$box
			?->value
	);

	f(
		(clone $box)
			?->value
	);

	f(
		isset($box)
		|| $b
	);

	f(
		isset($box)
		? $i
		: $s
	);

	f(
		Box::$flag
		|| $b
	);

	f(
		Box::$instance
			->get()
	);

	f(
		$box->next
			?->value ?? $i
	);

	f(
		$boxes[0]
			?->value ?? $i
	);

	f(
		(int) $s
		+ $i
	);

	f(
		$i++
		+ $i
	);

	f(
		count($boxes)
		+ $i
	);

	f(
		strlen($s)
		+ $i
	);

	f(
		$box::class
		. $s
	);

	f(
		$callable
		($i)
	);

	f(
		match ($i) {
			1 => 'one',
			2 => 'two',
			3 => 'three',
			4 => 'four',
			5 => 'five',
		}
	);
}

$box = new Box;
$box->next = new Box;
Box::$instance = $box;

test(false, false, $box, [$box], 4, '5', 'f');
