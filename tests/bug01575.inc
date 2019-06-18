<?php

class DebugClient01575 {

	// free port will be selected automatically by the operating system
	private $port = 0;

	private $tmpDir;

	public function __construct($port, $tmpDir) {
		$this->tmpDir = $tmpDir;
		$this->port = $port;
	}

	private function getAddress() {
		return "tcp://127.0.0.1:".$this->port;
	}

	public function startListen()
	{
		$socket = @stream_socket_server( $this->getAddress(), $errno, $errstr );
		if ( $socket === false )
		{
			echo "Could not create socket server - already in use?\n";
			echo "Error: {$errstr}, errno: {$errno}\n";
			echo "Address: {$this->getAddress()}\n";
			return;
		}

		while($conn = @stream_socket_accept( $socket, 3 )) {
			// read header
			$this->doRead( $conn );
			fwrite( $conn, 'run -i 0' . "\0" );
		}
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

			if(preg_match( '@\sidekey="([^"]+?)"@', $read, $matches )) {
				echo (strpos($matches[1], 'NEW_IDE_KEY') === false ? 'default' : $matches[1]).PHP_EOL;
			}

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
}

class DebugClient01575Manager {

	private static $process;
	private static $pipes;
	private static $tmpDir;

	public static function startListenOnBackground($listenPosrt, $tmpDir) {
		if(self::$process !== null) {
			self::stopListenOnBackground();
		}

		self::$tmpDir = $tmpDir;

		$currentFile = __FILE__;
		$code = <<<EOF
<?php
	require_once('$currentFile');
	\$dbgc = new DebugClient01575($listenPosrt, '$tmpDir'	);
	\$dbgc->startListen()
	
?>
EOF;

		file_put_contents( $tmpDir . '/xdebug-test01575-client.php',  $code);

		@unlink( $tmpDir . '/error-01575.txt' );
		@unlink( $tmpDir . '/output-01575.txt' );

		$descriptorspec = array(
			0 => array( 'pipe', 'r' ),
			1 => array( 'file', $tmpDir . '/output-01575.txt', 'a'  ),
			2 => array( 'file', $tmpDir . '/error-01575.txt', 'a' )
		);

		$php = getenv( 'TEST_PHP_EXECUTABLE' );
		$options = (getenv('TEST_PHP_ARGS') ?: '');
		$cmd = "{$php} $options {$tmpDir}/xdebug-test01575-client.php";
		$cwd = __DIR__;

		self::$process = proc_open( $cmd, $descriptorspec, self::$pipes, $cwd );
	}

	public static function stopListenOnBackground() {
		fclose( self::$pipes[0] );
		proc_close( self::$process );

		echo @file_get_contents(self::$tmpDir . '/output-01575.txt');
		echo @file_get_contents(self::$tmpDir . '/error-01575.txt');
	}

}

?>

