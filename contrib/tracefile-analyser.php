<?php
/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2018 Derick Rethans                               |
   +----------------------------------------------------------------------+
   | This source file is subject to version 1.01 of the Xdebug license,   |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | https://xdebug.org/license.php                                       |
   | If you did not receive a copy of the Xdebug license and are unable   |
   | to obtain it through the world-wide-web, please send a note to       |
   | derick@xdebug.org so we can mail you a copy immediately.             |
   +----------------------------------------------------------------------+
   | Authors: Derick Rethans <derick@xdebug.org>                          |
   +----------------------------------------------------------------------+
 */
if ( $argc <= 1 || $argc > 4 )
{
	showUsage();
}

$fileName = $argv[1];
$sortKey  = 'time-own';
$elements = 25;
if ( $argc > 2 )
{
	$sortKey = $argv[2];
	if ( !in_array( $sortKey, array( 'calls', 'time-inclusive', 'memory-inclusive', 'time-own', 'memory-own' ) ) )
	{
		showUsage();
	}
}
if ( $argc > 3 )
{
	$elements = (int) $argv[3];
}

$o = new drXdebugTraceFileParser( $argv[1] );
$o->parse();
$functions = $o->getFunctions( $sortKey );

// find longest function name
$maxLen = 0;
foreach( $functions as $name => $f )
{
	if ( strlen( $name ) > $maxLen )
	{
		$maxLen = strlen( $name );
	}
}
$maxLen = max( $maxLen, 8 );

echo "Showing the {$elements} most costly calls sorted by '{$sortKey}'.\n\n";

echo "        ", str_repeat( ' ', $maxLen - 8 ), "        Inclusive        Own\n";
echo "function", str_repeat( ' ', $maxLen - 8 ), "#calls  time     memory  time     memory\n";
echo "--------", str_repeat( '-', $maxLen - 8 ), "----------------------------------------\n";

// display functions
$c = 0;
foreach( $functions as $name => $f )
{
	$c++;
	if ( $c > $elements )
	{
		break;
	}
	printf( "%-{$maxLen}s %5d  %3.4f %8d  %3.4f %8d\n",
		$name, $f['calls'],
		$f['time-inclusive'], $f['memory-inclusive'],
		$f['time-own'], $f['memory-own'] );
}

function showUsage()
{
	echo "usage:\n\tphp run-cli tracefile [sortkey] [elements]\n\n";
	echo "Allowed sortkeys:\n\tcalls, time-inclusive, memory-inclusive, time-own, memory-own\n";
	die();
}

class drXdebugTraceFileParser
{
	protected $handle;

	/**
	 * Stores the last function, time and memory for the entry point per
	 * stack depth. int=>array(string, float, int).
	 */
	protected $stack;

	/**
	 * Stores per function the total time and memory increases and calls
	 * string=>array(float, int, int)
	 */
	protected $functions;

	/**
	 * Stores which functions are on the stack
	 */
	protected $stackFunctions;

	public function __construct( $fileName )
	{
		$this->handle = fopen( $fileName, 'r' );
		if ( !$this->handle )
		{
			throw new Exception( "Can't open '$fileName'" );
		}
		$this->stack[-1] = array( '', 0, 0, 0, 0 );
		$this->stack[ 0] = array( '', 0, 0, 0, 0 );

		$this->stackFunctions = array();
		$header1 = fgets( $this->handle );
		$header2 = fgets( $this->handle );
		if ( !preg_match( '@Version: [23].*@', $header1 ) || !preg_match( '@File format: [2-4]@', $header2 ) )
		{
			echo "\nThis file is not an Xdebug trace file made with format option '1' and version 2 to 4.\n";
			showUsage();
		}
	}

	public function parse()
	{
		echo "\nparsing...\n";
		$c = 0;
		$size = fstat( $this->handle );
		$size = $size['size'];
		$read = 0;
		
		while ( !feof( $this->handle ) )
		{
			$buffer = fgets( $this->handle, 4096 );
			$read += strlen( $buffer );
			$buffer = rtrim( $buffer, PHP_EOL );
			$this->parseLine( $buffer );
			$c++;

			if ( $c % 25000 === 0 )
			{
				printf( " (%5.2f%%)\n", ( $read / $size ) * 100 );
			}
		}
		echo "\nDone.\n\n";
	}

	private function parseLine( $line )
	{
	/*
		if ( preg_match( '@^Version: (.*)@', $line, $matches ) )
		{
		}
		else if ( preg_match( '@^File format: (.*)@', $line, $matches ) )
		{
		}
		else if ( preg_match( '@^TRACE.*@', $line, $matches ) )
		{
		}
		else // assume a normal line
		*/
		{
			$parts = explode( "\t", $line );
			if ( count( $parts ) < 5 )
			{
				return;
			}
			$depth = $parts[0];
			$funcNr = $parts[1];
			$time = $parts[3];
			$memory = $parts[4];
			if ( $parts[2] == '0' ) // function entry
			{
				$funcName = $parts[5];
				$intFunc = $parts[6];

				$this->stack[$depth] = array( $funcName, $time, $memory, 0, 0 );

				array_push( $this->stackFunctions, $funcName );
			}
			else if ( $parts[2] == '1' ) // function exit
			{
				list( $funcName, $prevTime, $prevMem, $nestedTime, $nestedMemory ) = $this->stack[$depth];

				// collapse data onto functions array
				$dTime   = $time   - $prevTime;
				$dMemory = $memory - $prevMem;

				if ( ! array_key_exists( $depth - 1, $this->stack ) )
				{
					$this->stack[$depth - 1] = array( '', 0, 0, 0, 0 );
				}
				$this->stack[$depth - 1][3] += $dTime;
				$this->stack[$depth - 1][4] += $dMemory;

				array_pop( $this->stackFunctions );

				$this->addToFunction( $funcName, $dTime, $dMemory, $nestedTime, $nestedMemory );
			}
		}
	}

	protected function addToFunction( $function, $time, $memory, $nestedTime, $nestedMemory )
	{
		if ( !isset( $this->functions[$function] ) )
		{
			$this->functions[$function] = array( 0, 0, 0, 0, 0 );
		}

		$elem = &$this->functions[$function];
		$elem[0]++;
		if ( !in_array( $function, $this->stackFunctions ) ) {
			$elem[1] += $time;
			$elem[2] += $memory;
			$elem[3] += $nestedTime;
			$elem[4] += $nestedMemory;
		}
	}

	public function getFunctions( $sortKey = null )
	{
		$result = array();
		foreach ( $this->functions as $name => $function )
		{
			$result[$name] = array(
				'calls'                 => $function[0],
				'time-inclusive'        => $function[1],
				'memory-inclusive'      => $function[2],
				'time-children'         => $function[3],
				'memory-children'       => $function[4],
				'time-own'              => $function[1] - $function[3],
				'memory-own'            => $function[2] - $function[4]
			);
		}

		if ( $sortKey !== null )
		{
			uasort( $result, 
				function( $a, $b ) use ( $sortKey )
				{
					return ( $a[$sortKey] > $b[$sortKey] ) ? -1 : ( $a[$sortKey] < $b[$sortKey] ? 1 : 0 );
				}
			);
		}

		return $result;
	}
}
?>
