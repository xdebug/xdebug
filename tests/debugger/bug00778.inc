<?php
$xml = '<?xml version="1.0" encoding="UTF-8" ?>
<root>Xdebug rocks
</root>';
$xml = str_replace("\r\n", "\n", $xml);
$temp = simplexml_load_string($xml, "SimpleXMLElement", LIBXML_NOERROR);
echo "It works?";
var_dump($temp);
?>
