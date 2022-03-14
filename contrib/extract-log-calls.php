<?php
$files = glob( $argv[1] );

foreach ( $files as $file )
{
	extractLogCallsFromFile( json_decode( file_get_contents( $file ) ) );
}

function extractLogCallsFromFile( $ast )
{
	walkNode( $ast );
}

function walkNode( $ast )
{
	$categories = [
		"CFG-", "LOG-", "DBG-", "GC-", "PROF-", "TRACE-", "COV-",
		"BASE-"
	];
	$levels = [
		"C", "E", "", "W", "", "", "", "I", "", "", "D"
	];

	if ( !isset( $ast->kind ) )
	{
		return;
	}

	if ( $ast->kind == 'CallExpr' )
	{
		@$name = $ast?->inner[0]?->inner[0]?->referencedDecl?->name;

		if ( $name == 'xdebug_log_ex' )
		{
			@$catValue = $ast?->inner[1]?->value;
			@$levelValue = $ast?->inner[2]?->value;
			if ( $levelValue == '' )
			{
				return;
			}
			echo array_key_exists( $catValue, $categories ) ? $categories[$catValue] : '???-';
			echo $levels[$levelValue], "-";
			@$value = trim( $ast?->inner[3]?->inner[0]?->inner[0]?->value, '"' );
			echo $value, ": ";
			@$value = trim( $ast?->inner[4]?->inner[0]?->inner[0]?->value, '"' );
			echo $value, "\n";
		}
	}
	if ( !isset ( $ast->inner ) )
	{
		return;
	}
	foreach ( $ast->inner as $child )
	{
		walkNode( $child );
	}
}
