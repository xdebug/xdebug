--TEST--
Test for bug #990: DBGP: Add notification for notices, warnings and errors
--SKIPIF--
<?php if (getenv("SKIP_DBGP_TESTS")) { exit("skip Excluding DBGp tests"); } ?>
--INI--
xdebug.remote_enable=1
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$data = file_get_contents(dirname(__FILE__) . '/bug00990-003.inc');

$commands = array(
	'step_into',
	'feature_set -n notify_ok -v 1',
	'step_into',
	'step_into',
	'detach'
);

dbgpRun( $data, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" fileuri="file:///tmp/xdebug-dbgp-test.php" language="PHP" xdebug:language_version="" protocol_version="1.0" appid="" idekey=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[http://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file:///tmp/xdebug-dbgp-test.php" lineno="2"></xdebug:message></response>

-> feature_set -i 2 -n notify_ok -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="2" feature="notify_ok" success="1"></response>

-> step_into -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file:///tmp/xdebug-dbgp-test.php" lineno="3"></xdebug:message></response>

-> step_into -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<notify xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" name="error" encoding="base64"><xdebug:message filename="file:///tmp/xdebug-dbgp-test.php" lineno="3" type_string="Fatal error"><![CDATA[Uncaught Error: Class 'MyClass' not found in /tmp/xdebug-dbgp-test.php:3
Stack trace:
#0 {main}
  thrown]]></xdebug:message><![CDATA[PGJyIC8+Cjxmb250IHNpemU9JzEnPjx0YWJsZSBjbGFzcz0neGRlYnVnLWVycm9yIHhlLXVuY2F1Z2h0LWV4Y2VwdGlvbicgZGlyPSdsdHInIGJvcmRlcj0nMScgY2VsbHNwYWNpbmc9JzAnIGNlbGxwYWRkaW5nPScxJz4KPHRyPjx0aCBhbGlnbj0nbGVmdCcgYmdjb2xvcj0nI2Y1NzkwMCcgY29sc3Bhbj0iNSI+PHNwYW4gc3R5bGU9J2JhY2tncm91bmQtY29sb3I6ICNjYzAwMDA7IGNvbG9yOiAjZmNlOTRmOyBmb250LXNpemU6IHgtbGFyZ2U7Jz4oICEgKTwvc3Bhbj4gRmF0YWwgZXJyb3I6IFVuY2F1Z2h0IEVycm9yOiBDbGFzcyAnTXlDbGFzcycgbm90IGZvdW5kIGluIC90bXAveGRlYnVnLWRiZ3AtdGVzdC5waHAgb24gbGluZSA8aT4zPC9pPjwvdGg+PC90cj4KPHRyPjx0aCBhbGlnbj0nbGVmdCcgYmdjb2xvcj0nI2Y1NzkwMCcgY29sc3Bhbj0iNSI+PHNwYW4gc3R5bGU9J2JhY2tncm91bmQtY29sb3I6ICNjYzAwMDA7IGNvbG9yOiAjZmNlOTRmOyBmb250LXNpemU6IHgtbGFyZ2U7Jz4oICEgKTwvc3Bhbj4gRXJyb3I6IENsYXNzICdNeUNsYXNzJyBub3QgZm91bmQgaW4gL3RtcC94ZGVidWctZGJncC10ZXN0LnBocCBvbi%s]]></notify>

<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="4" status="stopping" reason="ok"></response>

-> detach -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="detach" transaction_id="5" status="stopping" reason="ok"></response>
