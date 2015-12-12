<?php
require dirname(__FILE__) . '/../contrib/branch-coverage-to-dot.php';

function dump_branch_coverage($info)
{
	file_put_contents(sys_get_temp_dir() . "/paths.dot", branch_coverage_to_dot( $info ) );
	ksort($info);

	foreach ( $info as $fname => $file )
	{
		if ( preg_match( '/dump-branch-coverage.inc$/', $fname ) )
		{
			continue;
		}

		if ( !isset( $file['functions'] ) )
		{
			continue;
		}

		ksort( $file['functions'] );
		foreach ( $file['functions'] as $fname => $function )
		{

			if ( $fname == 'branch_coverage_to_dot' )
			{
				continue;
			}

			echo $fname, "\n", "- branches\n";
			foreach ( $function['branches'] as $bnr => $branch )
			{
				printf( "  - %02d; OP: %02d-%02d; line: %02d-%02d %3s",
					$bnr,
					$branch['op_start'], $branch['op_end'],
					$branch['line_start'], $branch['line_end'],
					$branch['hit'] ? "HIT" : " X "
				);
				if ( isset( $branch['out'][0] ) ) 
				{
					if ( $branch['out'][0] == 2147483645 )
					{
						printf("; out1: EX %3s",
							$branch['out_hit'][0] ? "HIT" : " X "
						);
					}
					else
					{
						printf("; out1: %02d %3s",
							$branch['out'][0],
							$branch['out_hit'][0] ? "HIT" : " X "
						);
					}
				}
				if ( isset( $branch['out'][1] ) ) 
				{
					if ( $branch['out'][1] == 2147483645 )
					{
						printf("; out2: EX %3s",
							$branch['out_hit'][1] ? "HIT" : " X "
						);
					}
					else
					{
						printf("; out2: %02d %3s",
							$branch['out'][1],
							$branch['out_hit'][1] ? "HIT" : " X "
						);
					}
				}
				echo "\n";
			}

			echo "- paths\n";
			foreach( $function['paths'] as $path )
			{
				echo '  - ', join( " ", $path['path'] ), ': ';
				echo $path['hit'] ? "HIT\n" : " X \n";
			}
			echo "\n";
		}
	}
}
