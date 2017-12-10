--TEST--
Test for bug #1501: Xdebug var dump tries casting properties (text)
--SKIPIF--
<?php if (PHP_INT_SIZE != 8) { echo "skip Only for 64bit platforms"; } ?>
--INI--
html_errors=0
xdebug.cli_color=0
xdebug.default_enable=1
xdebug.overload_var_dump=2
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
%sbug01501-001-64bit.php:20:
class h#%d (%d) {
  public ${373556941768884244} =>
  string(5) "hallo"
}
