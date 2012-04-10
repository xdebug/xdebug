<?php
$xml = '<?xml version="1.0" encoding="UTF-8" ?>
<root>
</root>';
$temp = simplexml_load_string($xml, "SimpleXMLElement", LIBXML_NOERROR);
echo "It works?";
?>
