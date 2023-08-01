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
class DOMDocument#1 (%d) {
  public $config =>
  NULL
  public $doctype =>
  NULL
  public $implementation =>
  string(22) "(object value omitted)"
  public $documentElement =>
  string(22) "(object value omitted)"
  public $actualEncoding =>
  NULL
  public $encoding =>
  NULL
  public $xmlEncoding =>
  NULL
  public $standalone =>
  bool(%s)
  public $xmlStandalone =>
  bool(%s)
  public $version =>
  string(3) "1.0"
  public $xmlVersion =>
  string(3) "1.0"
  public $strictErrorChecking =>
  bool(true)
  public $documentURI =>
  string(%d) "%s"
  public $formatOutput =>
  bool(false)
  public $validateOnParse =>
  bool(false)
  public $resolveExternals =>
  bool(false)
  public $preserveWhiteSpace =>
  bool(true)
  public $recover =>
  bool(false)
  public $substituteEntities =>
  bool(false)
  public $firstElementChild =>
  string(22) "(object value omitted)"
  public $lastElementChild =>
  string(22) "(object value omitted)"
  public $childElementCount =>
  int(1)
  public $nodeName =>
  string(9) "#document"
  public $nodeValue =>
  NULL
  public $nodeType =>
  int(9)
  public $parentNode =>
  %A
  public $childNodes =>
  string(22) "(object value omitted)"
  public $firstChild =>
  string(22) "(object value omitted)"
  public $lastChild =>
  string(22) "(object value omitted)"
  public $previousSibling =>
  NULL
  public $nextSibling =>
  NULL
  public $attributes =>
  %A
  public $ownerDocument =>
  NULL
  public $namespaceURI =>
  NULL
  public $prefix =>
  string(0) ""
  public $localName =>
  NULL
  public $baseURI =>
  string(%d) "%s"
  public $textContent =>
  string(4) "Test"
}
%sbug00913-php81.php:%d:
class DOMElement#2 (%d) {
  public $schemaTypeInfo =>
  NULL
  public $tagName =>
  %A
  public $firstElementChild =>
  NULL
  public $lastElementChild =>
  NULL
  public $childElementCount =>
  int(0)
  public $previousElementSibling =>
  NULL
  public $nextElementSibling =>
  NULL
  public $nodeName =>
  string(7) "example"
  public $nodeValue =>
  string(4) "Test"
  public $nodeType =>
  int(1)
  public $parentNode =>
  %A
  public $childNodes =>
  string(22) "(object value omitted)"
  public $firstChild =>
  string(22) "(object value omitted)"
  public $lastChild =>
  string(22) "(object value omitted)"
  public $previousSibling =>
  NULL
  public $nextSibling =>
  NULL
  public $attributes =>
  %A
  public $ownerDocument =>
  string(22) "(object value omitted)"
  public $namespaceURI =>
  NULL
  public $prefix =>
  string(0) ""
  public $localName =>
  string(7) "example"
  public $baseURI =>
  string(%d) "%s"
  public $textContent =>
  string(4) "Test"
}
%sbug00913-php81.php:%d:
class DOMAttr#3 (%d) {
  public $specified =>
  bool(true)
  public $schemaTypeInfo =>
  NULL
  public $name =>
  string(1) "a"
  public $value =>
  string(1) "b"
  public $ownerElement =>
  string(22) "(object value omitted)"
  public $nodeName =>
  string(1) "a"
  public $nodeValue =>
  string(1) "b"
  public $nodeType =>
  int(2)
  public $parentNode =>
  %A
  public $childNodes =>
  string(22) "(object value omitted)"
  public $firstChild =>
  string(22) "(object value omitted)"
  public $lastChild =>
  string(22) "(object value omitted)"
  public $previousSibling =>
  NULL
  public $nextSibling =>
  NULL
  public $attributes =>
  %A
  public $ownerDocument =>
  string(22) "(object value omitted)"
  public $namespaceURI =>
  NULL
  public $prefix =>
  string(0) ""
  public $localName =>
  string(1) "a"
  public $baseURI =>
  string(%d) "%s"
  public $textContent =>
  string(1) "b"
}
%sbug00913-php81.php:%d:
class DOMText#4 (%d) {
  public $wholeText =>
  string(4) "Test"
  public $data =>
  string(4) "Test"
  public $length =>
  int(4)
  public $previousElementSibling =>
  NULL
  public $nextElementSibling =>
  NULL
  public $nodeName =>
  string(5) "#text"
  public $nodeValue =>
  string(4) "Test"
  public $nodeType =>
  int(3)
  public $parentNode =>
  %A
  public $childNodes =>
  string(22) "(object value omitted)"
  public $firstChild =>
  NULL
  public $lastChild =>
  NULL
  public $previousSibling =>
  NULL
  public $nextSibling =>
  NULL
  public $attributes =>
  %A
  public $ownerDocument =>
  string(22) "(object value omitted)"
  public $namespaceURI =>
  NULL
  public $prefix =>
  string(0) ""
  public $localName =>
  NULL
  public $baseURI =>
  string(%d) "%s"
  public $textContent =>
  string(4) "Test"
}
