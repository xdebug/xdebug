--TEST--
Test for bug #747: Xdebug and SOAP Server conflict (>= PHP 5.3)
--SKIPIF--
<?php if (!version_compare(phpversion(), "5.3", '>=')) echo "skip >= PHP 5.3 needed\n"; ?>
--INI--
xdebug.default_enable=1
--FILE--
<?php
try {
	  $sc = new SoapServer("some-wrong.wsdl", array('exceptions' => true));
} catch (Exception $e) {
	  echo 'Error Caught :-)';
}
?>
--EXPECTF--
<?xml version="1.0" encoding="UTF-8"?>
<SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/"><SOAP-ENV:Body><SOAP-ENV:Fault><faultcode>WSDL</faultcode><faultstring>SOAP-ERROR: Parsing WSDL: Couldn't load from 'some-wrong.wsdl' : failed to load external entity "some-wrong.wsdl"
</faultstring></SOAP-ENV:Fault></SOAP-ENV:Body></SOAP-ENV:Envelope>
