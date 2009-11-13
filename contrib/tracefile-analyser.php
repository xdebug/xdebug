<?php
require 'api/parser.php';

if ( $argc <= 1 || $argc > 4 )
{
	echo "usage:\n\tphp run-cli tracefile [sortkey] [elements]\n\n";
	echo "Allowed sortkeys:\n\tcalls, time-inclusive, memory-inclusive, time-own, memory-own\n";
	die();
}

$fileName = $argv[1];
$sortKey  = 'time-own';
$elements = 25;
if ( $argc > 2 )
{
	$sortKey = $argv[2];
}
if ( $argc > 3 )
{
	$elements = (int) $argv[3];
}

$o = new drXdebugTraceFileParser( $argv[1] );
$o->parse();
$functions = $o->getFunctions( $sortKey );

echo "Showing the {$elements} most costly calls sorted by '{$sortKey}'.\n\n";

echo "                                               Inclusive        Own\n";
echo "function                               #calls  time     memory  time     memory\n";
echo "-------------------------------------------------------------------------------\n";

$c = 0;
foreach( $functions as $name => $f )
{
	$c++;
	if ( $c > $elements )
	{
		break;
	}
	printf( "%-39s %5d  %3.4f %8d  %3.4f %8d\n",
		$name, $f['calls'],
		$f['time-inclusive'], $f['memory-inclusive'],
		$f['time-own'], $f['memory-own'] );
}

