--TEST--
Test for bug #801: Segfault with streamwrapper, unclosed $fp and xdebug on destruction
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
    public $context;
    protected $_fp;

    public function stream_close()
    {
        die('DONE');
    }

    public function stream_open($path, $mode)
    {
        $this->_fp = fopen(sys_get_temp_dir() . '/asdf', $mode);

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
