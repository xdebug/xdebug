<?php
class foo
{
	function getLoader()
	{
		$map = require __DIR__ . '/autoload_namespaces.php';
		foreach ($map as $namespace => $path) {
			$loader->set($namespace, $path);
		}

		$map = require __DIR__ . '/autoload_psr4.php';
		foreach ($map as $namespace => $path) {
			$loader->setPsr4($namespace, $path);
		}

		$classMap = require __DIR__ . '/autoload_classmap.php';
		if ($classMap) {
			$loader->addClassMap($classMap);
		}

		$loader->register(true);

		return $loader;
	}

}
