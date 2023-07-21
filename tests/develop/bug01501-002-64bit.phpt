--TEST--
Test for bug #1501: Xdebug var dump tries casting properties (HTML)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('64bit');
?>
--INI--
xdebug.mode=develop
html_errors=1
xdebug.cli_color=0
xdebug.file_link_format=
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
<pre class='xdebug-var-dump' dir='ltr'>
<small>%sbug01501-002-64bit.php:20:</small>
<b>object</b>(<i>h</i>)[<i>%d</i>]
  <i>public</i> 373556941768884244 <font color='#888a85'>=&gt;</font> <small>string</small> <font color='#cc0000'>'hallo'</font> <i>(length=5)</i>
</pre>
