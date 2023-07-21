--TEST--
Test for bug #609: Xdebug and SOAP error handler conflicts
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('ext SOAP');
?>
--INI--
xdebug.mode=develop
--FILE--
<?php
try {
	  $sc = new SoapClient("some-wrong.wsdl", array('exceptions' => true));
} catch (Exception $e) {
	  echo 'Error Caught';
}
?>
--EXPECTF--
Error Caught
