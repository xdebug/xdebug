<?php
function branch_coverage_to_dot( $info, $pathInsteadOfBranch = true )
{
	$output = '';

	$c = 0;

	$output .= "digraph {\n";

	ksort($info);
	foreach ( $info as $fname => $file )
	{
		if ( preg_match( '/dump-branch-coverage.inc$/', $fname ) )
		{
			continue;
		}
		if ( preg_match( '/branch-coverage-to-dot.php$/', $fname ) )
		{
			continue;
		}

		if ( !isset( $file['functions'] ) )
		{
			continue;
		}

		$output .= sprintf("subgraph cluster_file_%s {\nlabel=\"%s\";\n", md5($fname), $fname);

		ksort( $file['functions'] );
		foreach ( $file['functions'] as $fname => $function )
		{
			$output .= sprintf("subgraph cluster_%s {\n\tlabel=\"%s\";\n\tgraph [rankdir=\"LR\"];\n\tnode [shape = record];\n", md5($fname), $fname);

			foreach ( $function['branches'] as $bnr => $branch )
			{
				$output .= sprintf( "\t\"__%s_%d\" [ label = \"{ op #%d-%d | line %d-%d }\" ];\n",
					$fname, $bnr,
					$branch['op_start'], $branch['op_end'],
					$branch['line_start'], $branch['line_end']
				);

				if ( ! $pathInsteadOfBranch )
				{
					if ( isset( $branch['out'][0] ) ) 
					{
						$output .= sprintf( "\t\"__%s_%d\" -> \"__%s_%d\" %s;\n",
							$fname, $bnr, $fname, $branch['out'][0],
							$branch['out_hit'][0] ? '' : '[style=dashed]'
						);
					}
					if ( isset( $branch['out'][1] ) ) 
					{
						$output .= sprintf( "\t\"__%s_%d\" -> \"__%s_%d\" %s;\n",
							$fname, $bnr, $fname, $branch['out'][1],
							$branch['out_hit'][1] ? '' : '[style=dashed]'
						);
					}
				}
			}

			if ( $pathInsteadOfBranch )
			{
				$output .= sprintf( "\t\"__%s_ENTRY\" [label=\"ENTRY\"];", $fname );
				$output .= sprintf( "\t\"__%s_EXIT\" [label=\"EXIT\"];", $fname );
				foreach( $function['paths'] as $path )
				{
					$output .= sprintf( "\t\"__%s_ENTRY\" -> \"__%s_%d\"",
							$fname, $fname, $path['path'][0]
					);
					for ( $i = 1; $i < sizeof( $path['path'] ); $i++ )
					{
						$output .= sprintf( " -> \"__%s_%d\"",
							$fname, $path['path'][$i]
						);
					}
					$lastOp = $path['path'][sizeof($path['path']) - 1];

					if ( isset( $function['branches'][$lastOp]['out'][0] ) && $function['branches'][$lastOp]['out'][0] == 2147483645 )
					{
						$output .= sprintf( " -> \"__%s_EXIT\"", $fname );
					}
					if ( isset( $function['branches'][$lastOp]['out'][1] ) && $function['branches'][$lastOp]['out'][1] == 2147483645 )
					{
						$output .= sprintf( " -> \"__%s_EXIT\"", $fname );
					}
					$output .= sprintf( " [color=\"/set19/%d\" penwidth=3 %s];\n",
						($c % 9) + 1,
						$path['hit'] ? '' : ' style=dashed'
					);
					$c++;
				}
			}

			$output .= "}\n";
		}

		$output .= "}\n";
	}

	$output .= "}\n";

	return $output;
}
