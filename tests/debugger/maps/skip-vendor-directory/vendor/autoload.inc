<?php
class Loader
{
	static function loadClass(string $name)
	{
		$classFile =  __DIR__ . DIRECTORY_SEPARATOR . '..' . DIRECTORY_SEPARATOR . str_replace('\\', DIRECTORY_SEPARATOR, strtolower($name)) . '.inc';
		require $classFile;
	}
}

spl_autoload_register('Loader::loadClass');
