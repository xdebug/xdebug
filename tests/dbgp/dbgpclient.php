<?php
class DebugClient
{
	private function open()
	{
		$socket = @stream_socket_server("tcp://0.0.0.0:9991", $errno, $errstr);
		return $socket;
	}

	private function launchPhp( &$pipes, array $ini_options = null )
	{
		@unlink( '/tmp/error-output.txt' );
		@unlink( '/tmp/remote_log.txt' );

		$descriptorspec = array(
		   0 => array( 'pipe', 'r' ),
		   1 => array( 'pipe', 'w' ),
		   2 => array( 'file', '/tmp/error-output.txt', 'a' )
		);

		$options = '';

		if ( !is_null( $ini_options ) && count( $ini_options ) > 0 )
		{
			foreach ( $ini_options as $key => $value )
			{
				$options .= " -d{$key}=$value";
			}
		}

		$cmd = "php $options -dxdebug.remote_enable=1 -dxdebug.remote_autostart=1 -dxdebug.remote_port=9991 -dxdebug.remote_log=/tmp/remote_log.txt /tmp/xdebug-dbgp-test.php";
		$cwd = dirname( __FILE__ );

		$process = proc_open( $cmd, $descriptorspec, $pipes, $cwd );
		return $process;
	}

	function doRead( $conn )
	{
		stream_set_timeout( $conn, 0, 1000 );
		do {
			$end = true;
			do {
				$header = stream_get_line( $conn, 10240, "\0" );
			} while ( $header === false );
			$read   = stream_get_line( $conn, 102400, "\0" );

			// sanitize
			$read = preg_replace( '@\s(appid|id|address)="\d+?"@', ' \\1=""', $read );
			$read = preg_replace( '@\s(idekey)="[^"]+?"@', ' \\1=""', $read );
			$read = preg_replace( '@(engine\sversion)="[^"]+?"@', '\\1=""', $read );
			$read = preg_replace( '@(2002-20[0-9]{2})@', '2002-2099', $read );
			echo $read, "\n\n";

			if ( preg_match( '@<stream xmlns="urn.debugger_protocol_v1" xmlns:xdebug@', $read ) )
			{
				$end = false;
			}
		} while( !$end );
	}

	function runTest( $data, array $commands, array $ini_options = null )
	{
		file_put_contents( '/tmp/xdebug-dbgp-test.php', $data );
		$i = 1;
		$socket = $this->open();
		if ( $socket === false )
		{
			echo "Could not create socket server - already in use?\n";
			return;
		}
		$php = $this->launchPhp( $ppipes, $ini_options );
		$conn = @stream_socket_accept( $socket, 3 );

		if ( $conn === false )
		{
			echo @file_get_contents( '/tmp/error-output.txt' ), "\n";
			echo @file_get_contents( '/tmp/remote_log.txt' ), "\n";
			proc_close( $php );
			return;
		}

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

function dbgpRun( $data, $commands, array $ini_options = null)
{
	$t = new DebugClient;
	$t->runTest( $data, $commands, $ini_options );
}
?>
