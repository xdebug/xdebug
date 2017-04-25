<?php
class Xdebug
{
	static function foo($s)
	{
		echo strstr("Hello!\n", "e");
	}

	static function not_used()
	{
		echo "this function does not get called\n";
	}
}
