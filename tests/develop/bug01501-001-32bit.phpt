--TEST--
Test for bug #1501: Xdebug var dump tries casting properties (text)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('32bit');
?>
--INI--
html_errors=0
xdebug.cli_color=0
xdebug.mode=develop
--FILE--
<?php

class h extends stdClass {
    public $data = array();
    
    function __set($name, $val) {
        $this->data[$name] = $val;
    }
    
    function __debugInfo() {
        return $this->data;
    }
}

$cl = new h();

$id = "373556941768884244";
$cl->$id = 'hallo';

var_dump($cl);
?>
--EXPECTF--
%sbug01501-001-32bit.php:20:
class h#%d (%d) {
  public $373556941768884244 =>
  string(5) "hallo"
}
