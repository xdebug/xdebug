--TEST--
Test for bug #747: Xdebug and SOAP Server conflict
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('ext soap');
?>
--INI--
xdebug.mode=develop
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
