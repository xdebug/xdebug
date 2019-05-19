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
