--TEST--
Test for bug #913: "Added debug info handler to DOM objects" not supported (>= PHP 8.1)
--INI--
xdebug.mode=develop
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.1; class DOMDocument');
?>
--FILE--
<?php
$DOMDocumentNode = new DOMDocument();
$DOMDocumentNode->loadXML('<example a="b">Test</example>');
$DOMElementNode   = $DOMDocumentNode->documentElement;
$DOMAttributeNode = $DOMElementNode->getAttributeNode('a');
$DOMTextNode      = $DOMElementNode->firstChild;

error_reporting(0);
var_dump($DOMDocumentNode, $DOMElementNode, $DOMAttributeNode, $DOMTextNode);
?>
--EXPECTF--
%sbug00913-php81.php:%d:
class DOMDocument#1 (%d) {%A
}
%sbug00913-php81.php:%d:
class DOMElement#2 (%d) {%A
}
%sbug00913-php81.php:%d:
class DOMAttr#3 (%d) {%A
}
%sbug00913-php81.php:%d:
class DOMText#4 (%d) {%A
}
