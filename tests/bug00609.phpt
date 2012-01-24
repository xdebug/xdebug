--TEST--
Test for bug #609: Xdebug and SOAP error handler conflicts (< PHP 5.3)
--SKIPIF--
<?php if (!version_compare(phpversion(), "5.3", '<')) echo "skip < PHP 5.3 needed\n"; ?>
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
Warning: SoapClient::%s(): I/O warning : failed to load external entity "some-wrong.wsdl" in %sbug00609.php on line 3
Error Caught :-)
