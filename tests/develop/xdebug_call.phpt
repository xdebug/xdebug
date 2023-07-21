--TEST--
Test for xdebug_call_*()
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
		echo $var, ': ', xdebug_call_class(), '>', xdebug_call_function(), ' @ ', xdebug_call_file(), ':', xdebug_call_line(), "\n";
		c( $var + 1);
	}

	public function aa( $var )
	{
		echo $var, ': ', xdebug_call_class(), '>', xdebug_call_function(), ' @ ', xdebug_call_file(), ':', xdebug_call_line(), "\n";
		a::b( $var + 1 );
	}

	static public function b( $var )
	{
		echo $var, ': ', xdebug_call_class(), '>', xdebug_call_function(), ' @ ', xdebug_call_file(), ':', xdebug_call_line(), "\n";
		c( $var + 1);
	}
}

function c( $var )
{
	echo $var, ': ', xdebug_call_class(), '>', xdebug_call_function(), ' @ ', xdebug_call_file(), ':', xdebug_call_line(), "\n";
	d( $var + 1 );
}

function d( $var )
{
	echo $var, ': ', xdebug_call_class(), '>', xdebug_call_function(), ' @ ', xdebug_call_file(), ':', xdebug_call_line(), "\n";
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
1: >{main} @ %sxdebug_call.php:0

1: >{main} @ %sxdebug_call.php:0
2: >c @ %sxdebug_call.php:35

1: >{main} @ %sxdebug_call.php:0
2: a>b @ %sxdebug_call.php:37
3: >c @ %sxdebug_call.php:18

1: >{main} @ %sxdebug_call.php:0
2: a>__construct @ %sxdebug_call.php:39
3: >c @ %sxdebug_call.php:6

1: >{main} @ %sxdebug_call.php:0
2: a>aa @ %sxdebug_call.php:41
3: a>b @ %sxdebug_call.php:12
4: >c @ %sxdebug_call.php:18
