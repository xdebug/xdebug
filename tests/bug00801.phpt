--TEST--
Test for bug #801: Segfault with streamwrapper, unclosed $fp and xdebug on destruction
--SKIPIF--
<?php
	// PHP 5.1 and PHP 5.2 both crash here, without Xdebug loaded
	if (!version_compare(phpversion(), "5.3", '>=')) echo "skip >= PHP 5.3 needed\n";
?>
--FILE--
<?php

class Handler
{
    protected $_sp;

    public function __construct()
    {
        $this->_sp = fopen('wrap://lalal', 'a');
    }
}

class Wrapper
{
    protected $_fp;

    public function stream_close()
    {
        die('DONE');
    }

    public function stream_open($path, $mode)
    {
        $this->_fp = fopen('/tmp/asdf', $mode);

        return true;
    }
}

stream_wrapper_register('wrap', 'Wrapper');

class Manager
{
    public static $stuff;
}

Manager::$stuff = new stdClass();
Manager::$stuff->Logs = new Handler;
?>
--EXPECT--
DONE
