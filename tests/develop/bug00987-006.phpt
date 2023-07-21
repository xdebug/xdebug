--TEST--
Test for bug #987: Hidden property names not shown with var_dump
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('ext simplexml');
?>
--INI--
html_errors=0
xdebug.mode=develop
xdebug.cli_color=0
error_reporting=-1
xdebug.show_local_vars=0
--FILE--
<?php
$xml = '<?xml version="1.0" encoding="UTF-8" ?>
<root>
Xdebug rocks
</root>';
$temp = simplexml_load_string($xml, "SimpleXMLElement", LIBXML_NOERROR);
var_dump($temp);
?>
--EXPECTF--
%sbug00987-006.php:7:
class SimpleXMLElement#%d (1) {
  public ${0} =>
  string(14) "
Xdebug rocks
"
}
