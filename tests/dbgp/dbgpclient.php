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
		return "0.0.0.0";
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

	private function launchPhp( &$pipes, array $ini_options = null )
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
			"xdebug.remote_log" => "{$this->tmpDir}/remote_log.txt"
		);

		if ( is_null( $ini_options ) )
		{
			$ini_options = array();
		}
		
		$options = '';
		$ini_options = array_merge( $default_options, $ini_options );
		foreach ( $ini_options as $key => $value )
		{
			$options .= " -d{$key}=$value";
		}

		$cmd = "php $options {$this->tmpDir}/xdebug-dbgp-test.php >{$this->tmpDir}/php-error-output.txt 2>&1";
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
			$read = preg_replace( '@\s(appid|id)="\d+?"@', ' \\1=""', $read );
			$read = preg_replace( '@\s(idekey)="[^"]+?"@', ' \\1=""', $read );
			$read = preg_replace( '@\s(xdebug:language_version)="[^"]+?"@', ' \\1=""', $read );
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
		file_put_contents( $this->tmpDir . '/xdebug-dbgp-test.php', $data );
		$i = 1;
		$socket = $this->open( $errno, $errstr );
		if ( $socket === false )
		{
			echo "Could not create socket server - already in use?\n";
			echo "Error: {$errstr}, errno: {$errno}\n";
			echo "Address: {$this->getAddress()}\n";
			return;
		}
		$php = $this->launchPhp( $ppipes, $ini_options );
		$conn = @stream_socket_accept( $socket, 3 );

		if ( $conn === false )
		{
			echo @file_get_contents( $this->tmpDir . '/php-error-output.txt' ), "\n";
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

			echo "-> ", $command, "\n";
			fwrite( $conn, $command . "\0" );

			$this->doRead( $conn );

			$i++;
		}
		fclose( $conn );
		fclose( $ppipes[0] );
		fclose( $ppipes[1] );
		proc_close( $php );
		
		// echo @file_get_contents( $this->tmpDir . '/php-error-output.txt' ), "\n";
		// echo @file_get_contents( $this->tmpDir . '/error-output.txt' ), "\n";
	}
}

class DebugClientIPv6 extends DebugClient
{
	protected function getIPAddress()
	{
		return "::";
	}

	protected function getAddress()
	{
		return "tcp://[" . $this->getIPAddress() . "]:" . $this->getPort();
	}

	public static function isSupported()
	{
		$ret = true;

		if ( !defined( "AF_INET6" ) )
		{
			return false;
		}
		
		$socket = socket_create( AF_INET6, SOCK_STREAM, SOL_TCP );

		if ( $socket === false )
		{
			return false;
		}
		
		if ( $ret && !socket_bind( $socket, $this->getIPAddress(), 9990 ) )
		{
			$ret = false;
		}

		if ( $ret && !socket_listen( $socket ) )
		{
			$ret = false;
		}

		socket_close( $socket );
		unset( $socket );

		return $ret;
	}
}

function dbgpRun( $data, $commands, array $ini_options = null, $flags = XDEBUG_DBGP_IPV4 )
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
