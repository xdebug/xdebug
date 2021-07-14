--TEST--
Test for bug #913: "Added debug info handler to DOM objects" not supported (>= PHP 8.1)
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
object(DOMDocument)#%d (%d) {
  ["config"]=>
  NULL
  ["doctype"]=>
  NULL
  ["implementation"]=>
  string(22) "(object value omitted)"
  ["documentElement"]=>
  string(22) "(object value omitted)"
  ["actualEncoding"]=>
  NULL
  ["encoding"]=>
  NULL
  ["xmlEncoding"]=>
  NULL
  ["standalone"]=>
  bool(true)
  ["xmlStandalone"]=>
  bool(true)
  ["version"]=>
  string(3) "1.0"
  ["xmlVersion"]=>
  string(3) "1.0"
  ["strictErrorChecking"]=>
  bool(true)
  ["documentURI"]=>
  string(%d) "%s"
  ["formatOutput"]=>
  bool(false)
  ["validateOnParse"]=>
  bool(false)
  ["resolveExternals"]=>
  bool(false)
  ["preserveWhiteSpace"]=>
  bool(true)
  ["recover"]=>
  bool(false)
  ["substituteEntities"]=>
  bool(false)
  ["firstElementChild"]=>
  string(22) "(object value omitted)"
  ["lastElementChild"]=>
  string(22) "(object value omitted)"
  ["childElementCount"]=>
  int(1)
  ["nodeName"]=>
  string(9) "#document"
  ["nodeValue"]=>
  NULL
  ["nodeType"]=>
  int(9)
  ["parentNode"]=>
  NULL
  ["childNodes"]=>
  string(22) "(object value omitted)"
  ["firstChild"]=>
  string(22) "(object value omitted)"
  ["lastChild"]=>
  string(22) "(object value omitted)"
  ["previousSibling"]=>
  NULL
  ["nextSibling"]=>
  NULL
  ["attributes"]=>
  NULL
  ["ownerDocument"]=>
  NULL
  ["namespaceURI"]=>
  NULL
  ["prefix"]=>
  string(0) ""
  ["localName"]=>
  NULL
  ["baseURI"]=>
  string(%d) "%s"
  ["textContent"]=>
  string(4) "Test"
}
object(DOMElement)#%d (%d) {
  ["schemaTypeInfo"]=>
  NULL
  ["tagName"]=>
  string(7) "example"
  ["firstElementChild"]=>
  NULL
  ["lastElementChild"]=>
  NULL
  ["childElementCount"]=>
  int(0)
  ["previousElementSibling"]=>
  NULL
  ["nextElementSibling"]=>
  NULL
  ["nodeName"]=>
  string(7) "example"
  ["nodeValue"]=>
  string(4) "Test"
  ["nodeType"]=>
  int(1)
  ["parentNode"]=>
  string(22) "(object value omitted)"
  ["childNodes"]=>
  string(22) "(object value omitted)"
  ["firstChild"]=>
  string(22) "(object value omitted)"
  ["lastChild"]=>
  string(22) "(object value omitted)"
  ["previousSibling"]=>
  NULL
  ["nextSibling"]=>
  NULL
  ["attributes"]=>
  string(22) "(object value omitted)"
  ["ownerDocument"]=>
  string(22) "(object value omitted)"
  ["namespaceURI"]=>
  NULL
  ["prefix"]=>
  string(0) ""
  ["localName"]=>
  string(7) "example"
  ["baseURI"]=>
  string(%d) "%s"
  ["textContent"]=>
  string(4) "Test"
}
object(DOMAttr)#%d (%d) {
  ["specified"]=>
  bool(true)
  ["schemaTypeInfo"]=>
  NULL
  ["name"]=>
  string(1) "a"
  ["value"]=>
  string(1) "b"
  ["ownerElement"]=>
  string(22) "(object value omitted)"
  ["nodeName"]=>
  string(1) "a"
  ["nodeValue"]=>
  string(1) "b"
  ["nodeType"]=>
  int(2)
  ["parentNode"]=>
  string(22) "(object value omitted)"
  ["childNodes"]=>
  string(22) "(object value omitted)"
  ["firstChild"]=>
  string(22) "(object value omitted)"
  ["lastChild"]=>
  string(22) "(object value omitted)"
  ["previousSibling"]=>
  NULL
  ["nextSibling"]=>
  NULL
  ["attributes"]=>
  NULL
  ["ownerDocument"]=>
  string(22) "(object value omitted)"
  ["namespaceURI"]=>
  NULL
  ["prefix"]=>
  string(0) ""
  ["localName"]=>
  string(1) "a"
  ["baseURI"]=>
  string(%d) "%s"
  ["textContent"]=>
  string(1) "b"
}
object(DOMText)#%d (%d) {
  ["wholeText"]=>
  string(4) "Test"
  ["data"]=>
  string(4) "Test"
  ["length"]=>
  int(4)
  ["previousElementSibling"]=>
  NULL
  ["nextElementSibling"]=>
  NULL
  ["nodeName"]=>
  string(5) "#text"
  ["nodeValue"]=>
  string(4) "Test"
  ["nodeType"]=>
  int(3)
  ["parentNode"]=>
  string(22) "(object value omitted)"
  ["childNodes"]=>
  string(22) "(object value omitted)"
  ["firstChild"]=>
  NULL
  ["lastChild"]=>
  NULL
  ["previousSibling"]=>
  NULL
  ["nextSibling"]=>
  NULL
  ["attributes"]=>
  NULL
  ["ownerDocument"]=>
  string(22) "(object value omitted)"
  ["namespaceURI"]=>
  NULL
  ["prefix"]=>
  string(0) ""
  ["localName"]=>
  NULL
  ["baseURI"]=>
  string(%d) "%s"
  ["textContent"]=>
  string(4) "Test"
}
