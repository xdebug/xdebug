<?php
require_once __DIR__ . '/../../utils.inc';

class CtrlSocketClient
{
	private string $tempFileName;

	function __construct( string $identifier )
	{
		$this->tempFileName = getTmpFile( $identifier );
	}

	function __destruct()
	{
		@unlink( $this->tempFileName );
	}

	function runCommand( string $command )
	{
		$myPid = getmypid();

		file_put_contents( $this->tempFileName, <<<"PHP"
<?php
\$addr = 'unix://\x00xdebug-ctrl.{$myPid}';
\$c = stream_socket_client(\$addr, \$errno, \$errstr, 2);
stream_set_write_buffer(\$c, 0);
fwrite(\$c, '{$command}');

stream_set_timeout(\$c, 0, 500000);
\$data = fread(\$c, 4096);
fclose(\$c);

echo \$data;
usleep(100000);
PHP);

		$descriptorspec = array(
		   0 => array( 'pipe', 'r' ),
		   1 => array( 'pipe', 'w' ),
		   2 => array( 'pipe', 'w' ),
		);

		$php = getenv( 'TEST_PHP_EXECUTABLE' );
		$cmd = "{$php} {$this->tempFileName} {$myPid}";
		if (strtoupper(substr(PHP_OS, 0, 3)) !== 'WIN') {
			$cmd = "exec {$cmd}";
		}
		$cwd = dirname( __FILE__ );

		$process = proc_open( $cmd, $descriptorspec, $pipes, $cwd );

		$end = microtime(true) + 0.5;
		while (microtime(true) < $end) {
			usleep(1000);
		}

		$cmdOutput = stream_get_contents( $pipes[1] );
		$cmdOutput = preg_replace( '@(engine\sversion)="[^"]+?"@', '\\1=""', $cmdOutput );
		$cmdOutput = preg_replace( '@(<fileuri>)(.*)(</fileuri>)@', '\\1\\3', $cmdOutput );
		$cmdOutput = preg_replace( '@(<pid>)(.*)(</pid>)@', '\\1\\3', $cmdOutput );
		$cmdOutput = preg_replace( '@(<time>)(.*)(</time>)@', '\\1\\3', $cmdOutput );
		$cmdOutput = preg_replace( '@(<memory>)(.*)(</memory>)@', '\\1\\3', $cmdOutput );
		echo $cmdOutput;

		$cmdOutput = stream_get_contents( $pipes[2] );
		echo $cmdOutput;

		fclose( $pipes[0] );
		fclose( $pipes[1] );
		fclose( $pipes[2] );

		proc_close( $process );
	}
}
?>
