--TEST--
Test for bug #801: Segfault with streamwrapper, unclosed $fp and xdebug on destruction
--FILE--
<?php
require_once __DIR__ . '/../utils.inc';

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
        $this->_fp = fopen(getTmpFile('asdf'), $mode);

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
--AFTER--
<?php
require_once __DIR__ . '/../utils.inc';

unlink(getTmpFile('asdf'));
?>
--EXPECT--
DONE
