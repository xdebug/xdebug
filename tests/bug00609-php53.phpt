--TEST--
Test for bug #609: Xdebug and SOAP error handler conflicts
--SKIPIF--
<?php if (!extension_loaded("soap")) { echo "skip SOAP extension required\n"; } ?>
--INI--
xdebug.default_enable=1
--FILE--
<?php
try {
	  $sc = new SoapClient("some-wrong.wsdl", array('exceptions' => true));
} catch (Exception $e) {
	  echo 'Error Caught :-)';
}
?>
--EXPECTF--
Error Caught :-)
