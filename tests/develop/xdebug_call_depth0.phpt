--TEST--
Test for xdebug_call_*(0)
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
		echo $var, ': ', xdebug_call_class(0), '>', xdebug_call_function(0), ' @ ', xdebug_call_file(0), ':', xdebug_call_line(0), "\n";
		c( $var + 1);
	}

	public function aa( $var )
	{
		echo $var, ': ', xdebug_call_class(0), '>', xdebug_call_function(0), ' @ ', xdebug_call_file(0), ':', xdebug_call_line(0), "\n";
		a::b( $var + 1 );
	}

	static public function b( $var )
	{
		echo $var, ': ', xdebug_call_class(0), '>', xdebug_call_function(0), ' @ ', xdebug_call_file(0), ':', xdebug_call_line(0), "\n";
		c( $var + 1);
	}
}

function c( $var )
{
	echo $var, ': ', xdebug_call_class(0), '>', xdebug_call_function(0), ' @ ', xdebug_call_file(0), ':', xdebug_call_line(0), "\n";
	d( $var + 1 );
}

function d( $var )
{
	echo $var, ': ', xdebug_call_class(0), '>', xdebug_call_function(0), ' @ ', xdebug_call_file(0), ':', xdebug_call_line(0), "\n";
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
1: >xdebug_call_function @ %sxdebug_call_depth0.php:30

1: >xdebug_call_function @ %sxdebug_call_depth0.php:24
2: >xdebug_call_function @ %sxdebug_call_depth0.php:30

1: >xdebug_call_function @ %sxdebug_call_depth0.php:17
2: >xdebug_call_function @ %sxdebug_call_depth0.php:24
3: >xdebug_call_function @ %sxdebug_call_depth0.php:30

1: >xdebug_call_function @ %sxdebug_call_depth0.php:5
2: >xdebug_call_function @ %sxdebug_call_depth0.php:24
3: >xdebug_call_function @ %sxdebug_call_depth0.php:30

1: >xdebug_call_function @ %sxdebug_call_depth0.php:11
2: >xdebug_call_function @ %sxdebug_call_depth0.php:17
3: >xdebug_call_function @ %sxdebug_call_depth0.php:24
4: >xdebug_call_function @ %sxdebug_call_depth0.php:30
