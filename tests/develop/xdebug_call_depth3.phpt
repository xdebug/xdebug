--TEST--
Test for xdebug_call_*(3)
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
		echo $var, ': ', xdebug_call_class(3), '>', xdebug_call_function(3), ' @ ', xdebug_call_file(3), ':', xdebug_call_line(3), "\n";
		c( $var + 1);
	}

	public function aa( $var )
	{
		echo $var, ': ', xdebug_call_class(3), '>', xdebug_call_function(3), ' @ ', xdebug_call_file(3), ':', xdebug_call_line(3), "\n";
		a::b( $var + 1 );
	}

	static public function b( $var )
	{
		echo $var, ': ', xdebug_call_class(3), '>', xdebug_call_function(3), ' @ ', xdebug_call_file(3), ':', xdebug_call_line(3), "\n";
		c( $var + 1);
	}
}

function c( $var )
{
	echo $var, ': ', xdebug_call_class(3), '>', xdebug_call_function(3), ' @ ', xdebug_call_file(3), ':', xdebug_call_line(3), "\n";
	d( $var + 1 );
}

function d( $var )
{
	echo $var, ': ', xdebug_call_class(3), '>', xdebug_call_function(3), ' @ ', xdebug_call_file(3), ':', xdebug_call_line(3), "\n";
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
1: > @ :

1: > @ :
2: >{main} @ %sxdebug_call_depth3.php:0

1: > @ :
2: >{main} @ %sxdebug_call_depth3.php:0
3: a>b @ %sxdebug_call_depth3.php:37

1: > @ :
2: >{main} @ %sxdebug_call_depth3.php:0
3: a>__construct @ %sxdebug_call_depth3.php:39

1: > @ :
2: >{main} @ %sxdebug_call_depth3.php:0
3: a>aa @ %sxdebug_call_depth3.php:41
4: a>b @ %sxdebug_call_depth3.php:12
