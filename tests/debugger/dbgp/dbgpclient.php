<?php
class DebugClient
{
	// free port will be selected automatically by the operating system
	protected $port = 0;

	private $tmpDir;

	protected $socket;
	protected $php;
	protected $ppipes;
	private   $pid = null;

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
		$envTmpDir = getenv('TEST_TMP_DIR');
		$this->tmpDir = $envTmpDir !== false ? $envTmpDir : sys_get_temp_dir();
		$this->tmpDir .= '/';
		$workerId = getenv( 'TEST_PHP_WORKER' );
		if ( $workerId !== false )
		{
			$this->tmpDir .= "{$workerId}-";
		}
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

	private function launchPhp( &$pipes, $filename, array $ini_options = [], array $extra_options = [] )
	{
		@unlink( $this->tmpDir . 'error-output.txt' );
		@unlink( $this->tmpDir . 'remote_log.txt' );

		$descriptorspec = array(
		   0 => array( 'pipe', 'r' ),
		   1 => array( 'pipe', 'w' ),
		   2 => array( 'file', $this->tmpDir . 'error-output.txt', 'a' )
		);

		$default_options = array(
			"xdebug.mode" => "debug",
			"xdebug.start_with_request" => "'yes'",
			"xdebug.client_host" => $this->getIPAddress(),
			"xdebug.client_port" => $this->getPort(),
		);

		$env_vars = array_key_exists( 'env', $extra_options ) ? $extra_options['env'] : [];
		$env_vars += $_ENV;

		$options = (getenv('TEST_PHP_ARGS') ?: '');
		$ini_options = array_merge( $default_options, $ini_options );
		foreach ( $ini_options as $key => $value )
		{
			$options .= " -d{$key}=$value";
		}

		if ( array_key_exists( 'auto_prepend', $extra_options ) )
		{
			$prependFile = "{$this->tmpDir}auto-prepend.inc";
			file_put_contents( $prependFile, $extra_options['auto_prepend'] );
			$options .= " -dauto_prepend_file={$prependFile}";
		}

		$php = getenv( 'TEST_PHP_EXECUTABLE' );
		$cmd = "{$php} $options {$filename} >{$this->tmpDir}php-stdout.txt 2>{$this->tmpDir}php-stderr.txt";
		if (strtoupper(substr(PHP_OS, 0, 3)) !== 'WIN') {
			$cmd = "exec {$cmd}";
		}
		$cwd = dirname( __FILE__ );

		$process = proc_open( $cmd, $descriptorspec, $pipes, $cwd, $env_vars );
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

	function doRead( $conn, string $transaction_id = null )
	{
		stream_set_timeout( $conn, 3 );
		do {
			$trans_id = null;
			$length = 0;
			while ( "\0" !== ( $char = fgetc($conn) ) ) {
				if ( $char === false ) {
					echo "read a false for $transaction_id" . PHP_EOL;
					return null;
				}
				if ( !is_numeric($char) ) {
					echo "read a non-number for $transaction_id" . PHP_EOL;
					return null;
				}
				$length = $length * 10 + (int)$char;
			}

			$read = '';
			while ( 0 < $length ) {
				$data = fread( $conn, $length );
				if ( $data === false )
				{
					echo "read a false for $transaction_id" . PHP_EOL;
					return null;
				}
				$length -= strlen( $data );
				$read .= $data;
			}
			$char = fgetc( $conn );
			if ( $char !== "\0" )
			{
				echo 'must end with \0' . PHP_EOL;
			}

			// sanitize
			$read = preg_replace( '@(?<!"Locals") id="\d+?(\d{4})"@', ' id="{{PID}}\\1"', $read );
			$read = preg_replace( '@\s(appid)="\d+?"@', ' \\1=""', $read );
			$read = preg_replace( '@\s(xdebug:language_version)="[^"]+?"@', ' \\1=""', $read );
			$read = preg_replace( '@(engine\sversion)="[^"]+?"@', '\\1=""', $read );
			$read = preg_replace( '@(2002-20[0-9]{2})@', '2002-2099', $read );
			$read = preg_replace_callback( '@\s(fileuri|filename)="file:///(.+?)"@', self::class . '::fixFilePath', $read );

			echo $read, "\n\n";

			if ( preg_match( '@<stream xmlns="urn.debugger_protocol_v1" xmlns:xdebug@', $read ) )
			{
				continue;
			}
			if ( preg_match( '@<notify xmlns="urn.debugger_protocol_v1" xmlns:xdebug@', $read ) )
			{
				continue;
			}
			$matches = [];
			if ( preg_match( '@transaction_id="(?P<transaction_id>\d+)"@', $read, $matches ) )
			{
				$trans_id = $matches['transaction_id'] ?? null;
			}
		} while ( $trans_id !== $transaction_id );
	}

	function start( $filename, array $ini_options = [], array $options = [])
	{
		$filename = realpath( $filename );

		$this->socket = $this->open( $errno, $errstr );
		if ( $this->socket === false )
		{
			echo "Could not create socket server - already in use?\n";
			echo "Error: {$errstr}, errno: {$errno}\n";
			echo "Address: {$this->getAddress()}\n";
			return false;
		}
		$this->php = $this->launchPhp( $this->ppipes, $filename, $ini_options, $options );
		$conn = @stream_socket_accept( $this->socket, isset( $options['timeout'] ) ? $options['timeout'] : 5 );

		if ( $conn === false )
		{
			echo @file_get_contents( $this->tmpDir . 'php-stdout.txt' ), "\n";
			echo @file_get_contents( $this->tmpDir . 'php-stderr.txt' ), "\n";
			echo @file_get_contents( $this->tmpDir . 'error-output.txt' ), "\n";
			echo @file_get_contents( $this->tmpDir . 'remote_log.txt' ), "\n";
			proc_close( $this->php );
			return false;
		}
		return $conn;
	}

	function stop( $conn, array $options = [] )
	{
		fclose( $conn );
		fclose( $this->ppipes[0] );
		fclose( $this->ppipes[1] );
		fclose( $this->socket );
		proc_close( $this->php );

		if ( array_key_exists( 'show-stdout', $options ) && $options['show-stdout'] )
		{
			echo @file_get_contents( $this->tmpDir . 'php-stdout.txt' ), "\n";
		}
		// echo @file_get_contents( $this->tmpDir . 'php-stderr.txt' ), "\n";
		// echo @file_get_contents( $this->tmpDir . 'error-output.txt' ), "\n";
	}

	function sendCommand( $conn, $command, $transaction_id )
	{
		// inject identifier
		$parts = explode( ' ', $command, 2 );
		if ( count($parts) == 1 )
		{
			$command = $parts[0] . " -i $transaction_id";
		}
		else
		{
			$command = $parts[0] . " -i $transaction_id " . $parts[1];
		}

		/* Replace PID macro */
		$command = str_replace( "{{PID}}", $this->pid & 0x1ffff, $command );

		$sanitised = $command;
		$sanitised = preg_replace( '@\sfile://.*[/\\\\](.*\.inc)\s@', ' file://\\1 ', $sanitised );

		echo "-> ", $sanitised, "\n";
		fwrite( $conn, $command . "\0" );
	}

	function runTest( $filename, array $commands, array $ini_options = [], array $options = [] )
	{
		$conn = $this->start( $filename, $ini_options, $options );
		if ( $conn === false )
		{
			return;
		}
		$i = 1;
		$procInfo = proc_get_status( $this->php );
		$this->pid = $procInfo['pid'];

		// read header
		$this->doRead( $conn );
		foreach ( $commands as $command )
		{
			$this->sendCommand( $conn, $command, $i );
			$this->doRead( $conn, (string)$i );

			$i++;
		}
		$this->stop( $conn, $options );
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

function dbgpRunFile( $data, $commands, array $ini_options = [], array $options = [] )
{
	if ( isset( $options['ipv'] ) && $options['ipv'] == 6 )
	{
		$t = new DebugClientIPv6();
	}
	else
	{
		$t = new DebugClient();
	}

	$t->runTest( $data, $commands, $ini_options, $options );
}
?>
