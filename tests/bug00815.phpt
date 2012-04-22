--TEST--
Test for bug #815: Xdebug crashes when 'exit' operator used in the script
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$data = file_get_contents(dirname(__FILE__) . '/bug00815.inc');

$commands = array(
	'feature_set -n show_hidden -v 1',
	'feature_set -n max_depth -v 1',
	'feature_set -n max_children -v 100',
	'status',
	'step_into',
	'eval -- aXNzZXQoJF9TRVJWRVJbJ1BIUF9JREVfQ09ORklHJ10p',
	'eval -- aXNzZXQoJF9TRVJWRVJbJ1NFUlZFUl9OQU1FJ10p',
	'eval -- KHN0cmluZykoJF9TRVJWRVJbJ1NFUlZFUl9OQU1FJ10p',
	'eval -- KHN0cmluZykoJF9TRVJWRVJbJ1NFUlZFUl9QT1JUJ10p',
	'eval -- KHN0cmluZykoJF9TRVJWRVJbJ1JFUVVFU1RfVVJJJ10p',
	'breakpoint_set -t line -n 2',
	'breakpoint_set -t line -n 2',
	'stack_get',
	'stack_get',
	'context_names',
	'context_get -c 0',
	'context_get -c 1',
	'run',
);

dbgpRun( $data, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" fileuri="file:///tmp/xdebug-dbgp-test.php" language="PHP" protocol_version="1.0" appid="" idekey=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[http://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2012 by Derick Rethans]]></copyright></init>

-> feature_set -i 1 -n show_hidden -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="1" feature="show_hidden" success="1"></response>

-> feature_set -i 2 -n max_depth -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="2" feature="max_depth" success="1"></response>

-> feature_set -i 3 -n max_children -v 100
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="3" feature="max_children" success="1"></response>

-> status -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="status" transaction_id="4" status="starting" reason="ok"></response>

-> step_into -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="5" status="break" reason="ok"><xdebug:message filename="file:///tmp/xdebug-dbgp-test.php" lineno="2"></xdebug:message></response>

-> eval -i 6 -- aXNzZXQoJF9TRVJWRVJbJ1BIUF9JREVfQ09ORklHJ10p
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="eval" transaction_id="6"><property address="" type="bool"><![CDATA[0]]></property></response>

-> eval -i 7 -- aXNzZXQoJF9TRVJWRVJbJ1NFUlZFUl9OQU1FJ10p
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="eval" transaction_id="7"><property address="" type="bool"><![CDATA[0]]></property></response>

-> eval -i 8 -- KHN0cmluZykoJF9TRVJWRVJbJ1NFUlZFUl9OQU1FJ10p
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="eval" transaction_id="8"><property address="" type="string" size="0" encoding="base64"><![CDATA[]]></property></response>

-> eval -i 9 -- KHN0cmluZykoJF9TRVJWRVJbJ1NFUlZFUl9QT1JUJ10p
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="eval" transaction_id="9"><property address="" type="string" size="0" encoding="base64"><![CDATA[]]></property></response>

-> eval -i 10 -- KHN0cmluZykoJF9TRVJWRVJbJ1JFUVVFU1RfVVJJJ10p
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="eval" transaction_id="10"><property address="" type="string" size="0" encoding="base64"><![CDATA[]]></property></response>

-> breakpoint_set -i 11 -t line -n 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="11" id=""></response>

-> breakpoint_set -i 12 -t line -n 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="12" id=""></response>

-> stack_get -i 13
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="13"><stack where="{main}" level="0" type="file" filename="file:///tmp/xdebug-dbgp-test.php" lineno="2"></stack></response>

-> stack_get -i 14
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="14"><stack where="{main}" level="0" type="file" filename="file:///tmp/xdebug-dbgp-test.php" lineno="2"></stack></response>

-> context_names -i 15
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="context_names" transaction_id="15"><context name="Locals" id=""></context><context name="Superglobals" id=""></context></response>

-> context_get -i 16 -c 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="16" context="0"><property name="$a" fullname="$a" type="uninitialized"></property></response>

-> context_get -i 17 -c 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="17" context="1"><property name="$_COOKIE" fullname="$_COOKIE" address="" type="array" children="0" numchildren="0" page="0" pagesize="100"></property><property name="$_ENV" fullname="$_ENV" address="" type="array" children="1" numchildren="44" page="0" pagesize="100"><property name="GNOME_KEYRING_PID" fullname="$_ENV[&#39;GNOME_KEYRING_PID&#39;]" address="" type="string" size="4" encoding="base64"><![CDATA[MzM4NQ==]]></property><property name="USER" fullname="$_ENV[&#39;USER&#39;]" address="" type="string" size="6" encoding="base64"><![CDATA[ZGVyaWNr]]></property><property name="LANGUAGE" fullname="$_ENV[&#39;LANGUAGE&#39;]" address="" type="string" size="8" encoding="base64"><![CDATA[ZW5fR0I6ZW4=]]></property><property name="SSH_AGENT_PID" fullname="$_ENV[&#39;SSH_AGENT_PID&#39;]" address="" type="string" size="4" encoding="base64"><![CDATA[MzQzOA==]]></property><property name="SHLVL" fullname="$_ENV[&#39;SHLVL&#39;]" address="" type="string" size="1" encoding="base64"><![CDATA[Mg==]]></property><property name="HOME" fullname="$_ENV[&#39;HOME&#39;]" address="" type="string" size="12" encoding="base64"><![CDATA[L2hvbWUvZGVyaWNr]]></property><property name="XDG_SESSION_COOKIE" fullname="$_ENV[&#39;XDG_SESSION_COOKIE&#39;]" address="" type="string" size="61" encoding="base64"><![CDATA[MGYyYjE3ZjczZjM1YjhkMzIyNDljYzIyMDAwMDAwMGItMTMzNDc4OTA1OC4zMDYyMzYtMTk3MTYyMzg3Ng==]]></property><property name="DESKTOP_SESSION" fullname="$_ENV[&#39;DESKTOP_SESSION&#39;]" address="" type="string" size="4" encoding="base64"><![CDATA[eGZjZQ==]]></property><property name="XDEBUG_CONFIG" fullname="$_ENV[&#39;XDEBUG_CONFIG&#39;]" address="" type="string" size="9" encoding="base64"><![CDATA[aWRla2V5PWRy]]></property><property name="DBUS_SESSION_BUS_ADDRESS" fullname="$_ENV[&#39;DBUS_SESSION_BUS_ADDRESS&#39;]" address="" type="string" size="72" encoding="base64"><![CDATA[dW5peDphYnN0cmFjdD0vdG1wL2RidXMtaGxUNkg1ZkNaMCxndWlkPWE1NjU0NDU1ZTM1MjYzYzhiMWFhZWVmNzAwMDAwMDI2]]></property><property name="GLADE_MODULE_PATH" fullname="$_ENV[&#39;GLADE_MODULE_PATH&#39;]" address="" type="string" size="1" encoding="base64"><![CDATA[Og==]]></property><property name="COLORTERM" fullname="$_ENV[&#39;COLORTERM&#39;]" address="" type="string" size="14" encoding="base64"><![CDATA[Z25vbWUtdGVybWluYWw=]]></property><property name="PATH_TRANSLATED" fullname="$_ENV[&#39;PATH_TRANSLATED&#39;]" address="" type="string" size="46" encoding="base64"><![CDATA[L2hvbWUvZGVyaWNrL2Rldi9waHAveGRlYnVnL3Rlc3RzL2J1ZzAwODE1LnBocA==]]></property><property name="SCRIPT_FILENAME" fullname="$_ENV[&#39;SCRIPT_FILENAME&#39;]" address="" type="string" size="46" encoding="base64"><![CDATA[L2hvbWUvZGVyaWNrL2Rldi9waHAveGRlYnVnL3Rlc3RzL2J1ZzAwODE1LnBocA==]]></property><property name="GNOME_KEYRING_CONTROL" fullname="$_ENV[&#39;GNOME_KEYRING_CONTROL&#39;]" address="" type="string" size="19" encoding="base64"><![CDATA[L3RtcC9rZXlyaW5nLU54SlNYcQ==]]></property><property name="LOGNAME" fullname="$_ENV[&#39;LOGNAME&#39;]" address="" type="string" size="6" encoding="base64"><![CDATA[ZGVyaWNr]]></property><property name="TEST_PHP_EXECUTABLE" fullname="$_ENV[&#39;TEST_PHP_EXECUTABLE&#39;]" address="" type="string" size="29" encoding="base64"><![CDATA[L3Vzci9sb2NhbC9waHAvNS4zZGV2L2Jpbi9waHA=]]></property><property name="WINDOWID" fullname="$_ENV[&#39;WINDOWID&#39;]" address="" type="string" size="8" encoding="base64"><![CDATA[MzY1NjQyOTY=]]></property><property name="_" fullname="$_ENV[&#39;_&#39;]" address="" type="string" size="29" encoding="base64"><![CDATA[L3Vzci9sb2NhbC9waHAvNS4zZGV2L2Jpbi9waHA=]]></property><property name="TERM" fullname="$_ENV[&#39;TERM&#39;]" address="" type="string" size="5" encoding="base64"><![CDATA[eHRlcm0=]]></property><property name="USERNAME" fullname="$_ENV[&#39;USERNAME&#39;]" address="" type="string" size="6" encoding="base64"><![CDATA[ZGVyaWNr]]></property><property name="WINDOWPATH" fullname="$_ENV[&#39;WINDOWPATH&#39;]" address="" type="string" size="1" encoding="base64"><![CDATA[Nw==]]></property><property name="PATH" fullname="$_ENV[&#39;PATH&#39;]" address="" type="string" size="54" encoding="base64"><![CDATA[L3Vzci9sb2NhbC9waHAvNS4zZGV2L2JpbjovdXNyL2xvY2FsL2JpbjovdXNyL2JpbjovYmlu]]></property><property name="GLADE_PIXMAP_PATH" fullname="$_ENV[&#39;GLADE_PIXMAP_PATH&#39;]" address="" type="string" size="1" encoding="base64"><![CDATA[Og==]]></property><property name="SESSION_MANAGER" fullname="$_ENV[&#39;SESSION_MANAGER&#39;]" address="" type="string" size="65" encoding="base64"><![CDATA[bG9jYWwvd2hpc2t5OkAvdG1wLy5JQ0UtdW5peC8zNDU4LHVuaXgvd2hpc2t5Oi90bXAvLklDRS11bml4LzM0NTg=]]></property><property name="XDG_MENU_PREFIX" fullname="$_ENV[&#39;XDG_MENU_PREFIX&#39;]" address="" type="string" size="5" encoding="base64"><![CDATA[eGZjZS0=]]></property><property name="DISPLAY" fullname="$_ENV[&#39;DISPLAY&#39;]" address="" type="string" size="4" encoding="base64"><![CDATA[OjAuMA==]]></property><property name="LANG" fullname="$_ENV[&#39;LANG&#39;]" address="" type="string" size="11" encoding="base64"><![CDATA[ZW5fR0IuVVRGLTg=]]></property><property name="MRT_DATA_DIR" fullname="$_ENV[&#39;MRT_DATA_DIR&#39;]" address="" type="string" size="31" encoding="base64"><![CDATA[L2hvbWUvZGVyaWNrL2luc3RhbGwvbW9kaXMvZGF0YQ==]]></property><property name="LS_COLORS" fullname="$_ENV[&#39;LS_COLORS&#39;]" address="" type="string" size="1302" encoding="base64"><![CDATA[cnM9MDpkaT0wMTszNDpsbj0wMTszNjptaD0wMDpwaT00MDszMzpzbz0wMTszNTpkbz0wMTszNTpiZD00MDszMzswMTpjZD00MDszMzswMTpvcj00MDszMTswMTpzdT0zNzs0MTpzZz0zMDs0MzpjYT0zMDs0MTp0dz0zMDs0Mjpvdz0zNDs0MjpzdD0zNzs0NDpleD0wMTszMjoqLnRhcj0wMTszMToqLnRnej0wMTszMToqLmFyaj0wMTszMToqLnRhej0wMTszMToqLmx6aD0wMTszMToqLmx6bWE9MDE7MzE6Ki50bHo9MDE7MzE6Ki50eHo9MDE7MzE6Ki56aXA9MDE7MzE6Ki56PTAxOzMxOiouWj0wMTszMToqLmR6PTAxOzMxOiouZ3o9MDE7MzE6Ki5sej0wMTszMToqLnh6PTAxOzMxOiouYnoyPTAxOzMxOiouYno9MDE7MzE6Ki50Yno9MDE7MzE6Ki50YnoyPTAxOzMxOioudHo9MDE7MzE6Ki5kZWI9MDE7MzE6Ki5ycG09MDE7MzE6Ki5qYXI9MDE7MzE6Ki53YXI9MDE7MzE6Ki5lYXI9MDE7MzE6Ki5zYXI9MDE7MzE6Ki5yYXI9MDE7MzE6Ki5hY2U9MDE7MzE6Ki56b289MDE7MzE6Ki5jcGlvPTAxOzMxOiouN3o9MDE7MzE6Ki5yej0wMTszMToqLmpwZz0wMTszNToqLmpwZWc9MDE7MzU6Ki5naWY9MDE7MzU6Ki5ibXA9MDE7MzU6Ki5wYm09MDE7MzU6Ki5wZ209MDE7MzU6Ki5wcG09MDE7MzU6Ki50Z2E9MDE7MzU6Ki54Ym09MDE7MzU6Ki54cG09MDE7MzU6Ki50aWY9MDE7MzU6Ki50aWZmPTAxOzM1OioucG5nPTAxOzM1Oiouc3ZnPTAxOzM1Oiouc3Znej0wMTszNToqLm1uZz0wMTszNToqLnBjeD0wMTszNToqLm1vdj0wMTszNToqLm1wZz0wMTszNToqLm1wZWc9MDE7MzU6Ki5tMnY9MDE7MzU6Ki5ta3Y9MDE7MzU6Ki53ZWJtPTAxOzM1Oioub2dtPTAxOzM1OioubXA0PTAxOzM1OioubTR2PTAxOzM1OioubXA0dj0wMTszNToqLnZvYj0wMTszNToqLnF0PTAxOzM1OioubnV2PTAxOzM1Oioud212PTAxOzM1OiouYXNmPTAxOzM1Oioucm09MDE7MzU6Ki5ybXZiPTAxOzM1OiouZmxjPTAxOzM1OiouYXZpPTAxOzM1OiouZmxpPTAxOzM1OiouZmx2PTAxOzM1OiouZ2w9MDE7MzU6Ki5kbD0wMTszNToqLnhjZj0wMTszNQ==]]></property><property name="XAUTHORITY" fullname="$_ENV[&#39;XAUTHORITY&#39;]" address="" type="string" size="45" encoding="base64"><![CDATA[L3Zhci9ydW4vZ2RtMy9hdXRoLWZvci1kZXJpY2stM2V2bmtVL2RhdGFiYXNl]]></property><property name="SSH_AUTH_SOCK" fullname="$_ENV[&#39;SSH_AUTH_SOCK&#39;]" address="" type="string" size="32" encoding="base64"><![CDATA[L3RtcC9zc2gtYk1zQkRYRlAzNDAzL2FnZW50LjM0MDM=]]></property><property name="GLADE_CATALOG_PATH" fullname="$_ENV[&#39;GLADE_CATALOG_PATH&#39;]" address="" type="string" size="1" encoding="base64"><![CDATA[Og==]]></property><property name="SHELL" fullname="$_ENV[&#39;SHELL&#39;]" address="" type="string" size="9" encoding="base64"><![CDATA[L2Jpbi9iYXNo]]></property><property name="GDMSESSION" fullname="$_ENV[&#39;GDMSESSION&#39;]" address="" type="string" size="4" encoding="base64"><![CDATA[eGZjZQ==]]></property><property name="REDIRECT_STATUS" fullname="$_ENV[&#39;REDIRECT_STATUS&#39;]" address

-> run -i 18
