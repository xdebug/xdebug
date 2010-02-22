<?php
class DebugClient
{
	private function open()
	{
		$socket = stream_socket_server("tcp://0.0.0.0:9991", $errno, $errstr);
		return $socket;
	}

	private function launchPhp( &$pipes )
	{
		$descriptorspec = array(
		   0 => array( 'pipe', 'r' ),
		   1 => array( 'pipe', 'w' ),
		   2 => array( 'file', '/tmp/error-output.txt', 'a' )
		);

		$cmd = "/usr/local/php/5.3dev/bin/php -dxdebug.remote_autostart=1 -dxdebug.remote_port=9991 /tmp/xdebug-dbgp-test.php";
		$cwd = dirname( __FILE__ );

		$process = proc_open( $cmd, $descriptorspec, $pipes, $cwd );
		return $process;
	}

	function doRead( $conn )
	{
		$read = trim( fread( $conn, 10240 ) );

		// sanitize
		$read = preg_replace( '@\s(id|address)="\d+?"@', ' \\1=""', $read );
		$parts = explode( "\0", $read, 2 );
		echo $parts[0], ': ', $parts[1], "\n\n";
	}

	function runTest( $data, array $commands )
	{
		file_put_contents( '/tmp/xdebug-dbgp-test.php', $data );
		$i = 1;
		$socket = $this->open();

		$php = $this->launchPhp( $ppipes );

		$conn = stream_socket_accept( $socket );

		// read header
		$this->doRead( $conn );
		foreach ( $commands as $command )
		{
			// inject identifier
			$parts = explode( ' ', $command, 2 );
			if ( count( $parts ) == 1 )
			{
				$command = $parts[0] . " -i $i";
			}
			else
			{
				$command = $parts[0] . " -i $i " . $parts[1];
			}

			echo "-> ", $command, "\n";
			fwrite( $conn, $command . "\0" );

			$this->doRead( $conn );

			$i++;
		}
		fclose( $conn );
		proc_close( $php );
	}
}

function dbgpRun( $data, $commands )
{
	$t = new DebugClient;
	$t->runTest( $data, $commands );
}
?>
