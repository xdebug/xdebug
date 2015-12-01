--TEST--
Test for bug #987: Hidden property names not shown with var_dump
--SKIPIF--
<?php if (!extension_loaded('simplexml')) echo "skip The SimpleXML extension needs to be installed\n"; ?>
--INI--
html_errors=0
xdebug.cli_color=0
xdebug.default_enable=1
error_reporting=-1
xdebug.collect_params=4
xdebug.show_local_vars=0
xdebug.overload_var_dump=1
--FILE--
<?php
$xml = '<?xml version="1.0" encoding="UTF-8" ?>
<root>
</root>';
$temp = simplexml_load_string($xml, "SimpleXMLElement", LIBXML_NOERROR);
var_dump($temp);
?>
--EXPECTF--
class SimpleXMLElement#1 (1) {
  public ${0} =>
  string(1) "
"
}
