<?php
function runtime_version( $value, $operator )
{
	return version_compare( phpversion(), $value, $operator );
}

function opcache_active()
{
	if (! extension_loaded( 'zend opcache' ) )
	{
		return false;
	}

	if ( ini_get( 'opcache.enable' ) == 0 )
	{
		return false;
	}

	if ( ini_get( 'opcache.enable_cli' ) == 0 )
	{
		return false;
	}

	if ( ini_get( 'opcache.optimization_level' ) === "0" )
	{
		return false;
	}

	return true;
}

function check_reqs( $req_str )
{
	$missing = 0;
	$errors = [];

	$parts = explode( ';', $req_str );

	foreach ( $parts as $key => $req )
	{
		$parts[$key] = $req = trim( $req );
		$old_missing = $missing;

		if ( $req === 'ZTS' )
		{
			$missing += ( PHP_ZTS == 0 );
		}
		if ( $req === 'NTS' )
		{
			$missing += ( PHP_ZTS == 1 );
		}
		if ( $req === 'opcache' )
		{
			$missing += ( !opcache_active() );
		}
		if ( $req === '!opcache' )
		{
			$missing += opcache_active();
		}

		if ( $req === 'linux' )
		{
			$missing += ( php_uname('s') != 'Linux' );
		}
		if ( $req === '!linux' )
		{
			$missing += ( php_uname('s') == 'Linux' );
		}

		if ( $req === 'sunos' )
		{
			$missing += ( php_uname('s') != 'SunOS' );
		}
		if ( $req === '!sunos' )
		{
			$missing += ( php_uname('s') == 'SunOS' );
		}

		if ( $req === 'win' )
		{
			$missing += ( substr( PHP_OS, 0, 3 ) !== 'WIN' );
		}
		if ( $req === '!win' )
		{
			$missing += ( substr( PHP_OS, 0, 3 ) === 'WIN' );
		}

		if ( $req === 'osx' )
		{
			$missing += ( PHP_OS !== 'Darwin' );
		}
		if ( $req === '!osx' )
		{
			$missing += ( PHP_OS === 'Darwin' );
		}

		if ( substr( $req, 0, 3 ) === 'PHP' )
		{
			$versions_req = explode( ',', substr( $req, 4 ) );
			foreach ( $versions_req as $version_req )
			{
				list( $operator, $version ) = explode( ' ', trim( $version_req ) );
				$missing += (int) ( ! runtime_version( $version, $operator ) );
			}
		}

		if ( substr( $req, 0, 9 ) === '!ext-flag' )
		{
			$missing += (int) ( in_array( substr( $req, 10 ), xdebug_info( 'extension-flags' ) ) );
			$req = "absence of 'xdebug-" . substr( $req, 10 ) . '\' compile flag';
		}

		if ( substr( $req, 0, 8 ) === 'ext-flag' )
		{
			$missing += (int) ( ! in_array( substr( $req, 9 ), xdebug_info( 'extension-flags' ) ) );
			$req = "'xdebug-" . substr( $req, 9 ) . '\' compile flag';
		}
		else if ( substr( $req, 0, 3 ) === 'ext' )
		{
			$missing += (int) ( ! extension_loaded( substr( $req, 4 ) ) );
			$req = "'" . substr( $req, 4 ) . '\' extension';
		}

		if ( substr( $req, 0, 5 ) === 'class' )
		{
			$missing += (int) ( in_array( substr( $req, 6 ), get_declared_classes()) == false );
			$req = "'" . substr( $req, 6 ) . '\' class';
		}

		if ( $req === '32bit' )
		{
			$missing += ( PHP_INT_SIZE != 4 );
		}
		if ( $req === '64bit' )
		{
			$missing += ( PHP_INT_SIZE != 8 );
		}

		if ( $req === 'unparallel' )
		{
			$missing += ( getenv( 'SKIP_UNPARALLEL_TESTS' ) );
			$missing += ( getenv( 'TEST_PHP_WORKER' ) !== false );
		}

		if ( $req === 'dbgp' )
		{
			$missing += (int) ( getenv( 'SKIP_DBGP_TESTS' ) );
			$req = "DBGp tests are disabled";
		}

		if ( $req === 'slow' )
		{
			$missing += (int) ( getenv( 'SKIP_SLOW_TESTS' ) );
			$req = "Ignoring slow tests";
		}

		if ( $old_missing != $missing )
		{
			$errors[] = $req;
		}
	}

	if ( $missing )
	{
		die( "skip required: " . join( ' && ', $parts ) . " > missing: " . join( ' && ', $errors ) );
	}
}

function mustBeExecuted( array $coverageInfo, array $lines )
{
	foreach ( $lines as $line )
	{
		if ( !array_key_exists( $line, $coverageInfo ) )
		{
			echo "FAIL: line #{$line} is not present in coverage info\n";
			continue;
		}

		if ( $coverageInfo[$line] !== 1 )
		{
			echo "FAIL: line #{$line} is not covered\n";
			continue;
		}

		echo "line #{$line} is present and covered\n";
	}
}

function mustNotBeExecuted( array $coverageInfo, array $lines )
{
	foreach ( $lines as $line )
	{
		if ( !array_key_exists( $line, $coverageInfo ) )
		{
			echo "FAIL: line #{$line} is not present in coverage info\n";
			continue;
		}

		if ( $coverageInfo[$line] === 1 )
		{
			echo "FAIL: line #{$line} is covered\n";
			continue;
		}

		echo "line #{$line} is present and not covered\n";
	}
}
?>
