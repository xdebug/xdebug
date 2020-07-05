--TEST--
Test for bug #1501: Xdebug var dump tries casting properties (ANSI)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('64bit');
?>
--INI--
xdebug.mode=develop
html_errors=0
xdebug.cli_color=2
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
[1m%sbug01501-003-64bit.php[22m:[1m20[22m:
[1mclass[22m [31mh[0m#%d ([32m%d[0m) {
  [32m[1mpublic[22m[0m ${373556941768884244} [0m=>[0m
  [1mstring[22m([32m5[0m) "[31mhallo[0m"
}
