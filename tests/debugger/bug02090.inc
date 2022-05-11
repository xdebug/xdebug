<?php
declare(strict_types=1);

class Test
{
	public static function __callStatic($name, $args)
	{
		echo "$name called\n";
	}
}

Test::works1();
Test::works2();

$ffi = FFI::cdef('int printf(const char *format, ...);', 'libc.so.6');
$ffi->printf("Hello %s!\n", "world");

Test::breaks();
