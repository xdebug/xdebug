--TEST--
Test for bug #1950: Assignment trace with ASSIGN_OBJ_REF crashes
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
?>
--INI--
xdebug.mode=trace
xdebug.start_with_request=0
xdebug.collect_return=0
xdebug.collect_assignments=1
xdebug.trace_format=0
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

final class SessionBag
{
    private $data;
	static private $dataAgain;

    public function __construct(array &$data)
    {
        $this->data = &$data;
    }

	static public function set(array &$data)
	{
		self::$dataAgain = &$data;
	}
}

$data = [ 'one', 'two', 'three' ];
$a = new SessionBag( $data );
SessionBag::set( $data );

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w             => $tf = '%s' %sbug01950.php:2
%w             => $data = [0 => 'one', 1 => 'two', 2 => 'three'] %sbug01950.php:20
%w%f %w%d     -> SessionBag->__construct($data = [0 => 'one', 1 => 'two', 2 => 'three']) %sbug01950.php:21
%w               => $this->data =& $data %sbug01950.php:11
%w             => $a = class SessionBag { private $data = [0 => 'one', 1 => 'two', 2 => 'three'] } %sbug01950.php:21
%w%f %w%d     -> SessionBag::set($data = [0 => 'one', 1 => 'two', 2 => 'three']) %sbug01950.php:22
%w               => self::dataAgain =& $data %sbug01950.php:16
%w%f %w%d     -> xdebug_stop_trace() %sbug01950.php:24
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
