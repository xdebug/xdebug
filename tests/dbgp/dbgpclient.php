<?php
define( 'XDEBUG_DBGP_IPV4', 1 );
define( 'XDEBUG_DBGP_IPV6', 2 );

class DebugClient
{
	// free port will be selected automatically by the operating system
	protected $port = 0;

	private $tmpDir;

	public function getPort()
	{
		return $this->port;
	}
	
	public function setPort($port)
	{
		$this->port = $port;
	}

	protected function getIPAddress()
	{
		return "127.0.0.1";
	}

	protected function getAddress()
	{
		return "tcp://" . $this->getIPAddress() . ":" . $this->getPort();
	}

	public function __construct()
	{
		$this->tmpDir = sys_get_temp_dir();
	}

	private function open( &$errno, &$errstr )
	{
		$socket = @stream_socket_server( $this->getAddress(), $errno, $errstr );
		if ( $socket )
		{
			$name = stream_socket_get_name( $socket, false );
			$name = explode( ":", $name );
			$this->port = array_pop( $name );
		}
		return $socket;
	}

	private function launchPhp( &$pipes, array $ini_options = null, $filename )
	{
		@unlink( $this->tmpDir . '/error-output.txt' );
		@unlink( $this->tmpDir . '/remote_log.txt' );

		$descriptorspec = array(
		   0 => array( 'pipe', 'r' ),
		   1 => array( 'pipe', 'w' ),
		   2 => array( 'file', $this->tmpDir . '/error-output.txt', 'a' )
		);
		
		$default_options = array(
			"xdebug.remote_enable" => "1",
			"xdebug.remote_autostart" => "1",
			"xdebug.remote_host" => $this->getIPAddress(),
			"xdebug.remote_port" => $this->getPort(),
			"xdebug.remote_log" => "{$this->tmpDir}/remote_log.txt",
			"xdebug.remote_log_level" => 10,
		);

		if ( is_null( $ini_options ) )
		{
			$ini_options = array();
		}
		
		$options = (getenv('TEST_PHP_ARGS') ?: '');
		$ini_options = array_merge( $default_options, $ini_options );
		foreach ( $ini_options as $key => $value )
		{
			$options .= " -d{$key}=$value";
		}

		$php = getenv( 'TEST_PHP_EXECUTABLE' );
		$cmd = "{$php} $options {$filename} >{$this->tmpDir}/php-stdout.txt 2>{$this->tmpDir}/php-stderr.txt";
		$cwd = dirname( __FILE__ );

		$process = proc_open( $cmd, $descriptorspec, $pipes, $cwd );
		return $process;
	}

	function fixFilePath( $m )
	{
		preg_match( '@.*/(.*\.inc)@', $m[2], $fm );
		if ( !isset( $fm[1] ) )
		{
			$fm[1] = '';
		}
		return " {$m[1]}=\"file://{$fm[1]}\"";
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
			$read = preg_replace( '@\s(appid|id)="\d+?"@', ' \\1=""', $read );
			$read = preg_replace( '@\s(idekey)="[^"]+?"@', ' \\1=""', $read );
			$read = preg_replace( '@\s(xdebug:language_version)="[^"]+?"@', ' \\1=""', $read );
			$read = preg_replace( '@(engine\sversion)="[^"]+?"@', '\\1=""', $read );
			$read = preg_replace( '@(2002-20[0-9]{2})@', '2002-2099', $read );
			$read = preg_replace_callback( '@\s(fileuri|filename)="file:///(.+?)"@', 'self::fixFilePath', $read );
			echo $read, "\n\n";

			if ( preg_match( '@<stream xmlns="urn.debugger_protocol_v1" xmlns:xdebug@', $read ) )
			{
				$end = false;
			}
			if ( preg_match( '@<notify xmlns="urn.debugger_protocol_v1" xmlns:xdebug@', $read ) )
			{
				$end = false;
			}
		} while( !$end );
	}

	function runTest( $filename, array $commands, array $ini_options = null )
	{
		$filename = realpath( $filename );

		$i = 1;
		$socket = $this->open( $errno, $errstr );
		if ( $socket === false )
		{
			echo "Could not create socket server - already in use?\n";
			echo "Error: {$errstr}, errno: {$errno}\n";
			echo "Address: {$this->getAddress()}\n";
			return;
		}
		$php = $this->launchPhp( $ppipes, $ini_options, $filename );
		$conn = @stream_socket_accept( $socket, 20 );

		if ( $conn === false )
		{
			echo @file_get_contents( $this->tmpDir . '/php-stdout.txt' ), "\n";
			echo @file_get_contents( $this->tmpDir . '/php-stderr.txt' ), "\n";
			echo @file_get_contents( $this->tmpDir . '/error-output.txt' ), "\n";
			echo @file_get_contents( $this->tmpDir . '/remote_log.txt' ), "\n";
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

			$sanitised = $command;
			$sanitised = preg_replace( '@\sfile://.*/(.*\.inc)\s@', ' file://\\1 ', $sanitised );

			echo "-> ", $sanitised, "\n";
			fwrite( $conn, $command . "\0" );

			$this->doRead( $conn );

			$i++;
		}
		fclose( $conn );
		fclose( $ppipes[0] );
		fclose( $ppipes[1] );
		fclose( $socket );
		proc_close( $php );
		
		// echo @file_get_contents( $this->tmpDir . '/php-stderr.txt' ), "\n";
		// echo @file_get_contents( $this->tmpDir . '/error-output.txt' ), "\n";
	}
}

class DebugClientIPv6 extends DebugClient
{
	protected function getIPAddress()
	{
		return "::1";
	}

	protected function getAddress()
	{
		return "tcp://[" . $this->getIPAddress() . "]:" . $this->getPort();
	}

	public static function isSupported( &$errno, &$errstr )
	{
		$socket = @stream_socket_server( "tcp://[::1]:0", $errno, $errstr );

		if ( $socket === false )
		{
			return false;
		}

		fclose( $socket );
		return true;
	}
}

function dbgpRunFile( $data, $commands, array $ini_options = null, $flags = XDEBUG_DBGP_IPV4 )
{
	if ( $flags == XDEBUG_DBGP_IPV6 )
	{
		$t = new DebugClientIPv6();
	}
	else
	{
		$t = new DebugClient();
	}

	$t->runTest( $data, $commands, $ini_options );
}
?>
