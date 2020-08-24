--TEST--
Test for xdebug_call_*(1)
--INI--
xdebug.mode=develop
xdebug.trace_options=0
xdebug.collect_return=0
xdebug.auto_profile=0
xdebug.dump_globals=0
xdebug.trace_format=0
--FILE--
<?php
class a {
	public function __construct( $var )
	{
		echo $var, ': ', xdebug_call_class(1), '>', xdebug_call_function(1), ' @ ', xdebug_call_file(1), ':', xdebug_call_line(1), "\n";
		c( $var + 1);
	}

	public function aa( $var )
	{
		echo $var, ': ', xdebug_call_class(1), '>', xdebug_call_function(1), ' @ ', xdebug_call_file(1), ':', xdebug_call_line(1), "\n";
		a::b( $var + 1 );
	}

	static public function b( $var )
	{
		echo $var, ': ', xdebug_call_class(1), '>', xdebug_call_function(1), ' @ ', xdebug_call_file(1), ':', xdebug_call_line(1), "\n";
		c( $var + 1);
	}
}

function c( $var )
{
	echo $var, ': ', xdebug_call_class(1), '>', xdebug_call_function(1), ' @ ', xdebug_call_file(1), ':', xdebug_call_line(1), "\n";
	d( $var + 1 );
}

function d( $var )
{
	echo $var, ': ', xdebug_call_class(1), '>', xdebug_call_function(1), ' @ ', xdebug_call_file(1), ':', xdebug_call_line(1), "\n";
}

d( 1 );
echo "\n";
c( 1 );
echo "\n";
a::b( 1 );
echo "\n";
$a = new a( 1 );
echo "\n";
$a->aa( 1 );
?>
--EXPECTF--
1: >d @ %sxdebug_call_depth1.php:33

1: >c @ %sxdebug_call_depth1.php:35
2: >d @ %sxdebug_call_depth1.php:25

1: a>b @ %sxdebug_call_depth1.php:37
2: >c @ %sxdebug_call_depth1.php:18
3: >d @ %sxdebug_call_depth1.php:25

1: a>__construct @ %sxdebug_call_depth1.php:39
2: >c @ %sxdebug_call_depth1.php:6
3: >d @ %sxdebug_call_depth1.php:25

1: a>aa @ %sxdebug_call_depth1.php:41
2: a>b @ %sxdebug_call_depth1.php:12
3: >c @ %sxdebug_call_depth1.php:18
4: >d @ %sxdebug_call_depth1.php:25
