--TEST--
Test for bug #1583: Xdebug crashes when OPcache's compact literals optimisation is on
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('ext json');
?>
--FILE--
<?php
class Foo
{
    public function __destruct() { $this->shutdown(); }
    public function shutdown($how=STREAM_SHUT_RDWR) { }
}

function get_it()
{
    return false;

}

$x = new Foo();
$x->shutdown();
echo json_encode(['x'=>get_it()]);
?>
--EXPECT--
{"x":false}
