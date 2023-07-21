<?php
require dirname(__FILE__) . '/../../contrib/branch-coverage-to-dot.php';

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
				$str = '';

				$str .= sprintf( "  - %02d; OP: %02d-%02d; line: %02d-%02d %3s",
					$bnr,
					$branch['op_start'], $branch['op_end'],
					$branch['line_start'], $branch['line_end'],
					$branch['hit'] ? "HIT" : " X "
				);
				foreach ( $branch['out'] as $key => $out )
				{
					if ( $out == 2147483645 )
					{
						$str .= sprintf("; out%d: EX %3s", $key + 1,
							$branch['out_hit'][$key] ? "HIT" : " X "
						);
					}
					else
					{
						$str .= sprintf("; out%d: %02d %3s", $key + 1,
							$branch['out'][$key],
							$branch['out_hit'][$key] ? "HIT" : " X "
						);
					}
				}
				echo rtrim( $str ). "\n";
			}

			echo "- paths\n";
			foreach( $function['paths'] as $path )
			{
				echo '  - ', join( " ", $path['path'] ), ': ';
				echo $path['hit'] ? "HIT\n" : " X\n";
			}
			echo "\n";
		}
	}
}
