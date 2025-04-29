--TEST--
Test for enums [1]
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.1');
?>
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.collect_return=0
xdebug.collect_assignments=1
xdebug.trace_format=1
--FILE--
<?php
require_once 'capture-trace.inc';

enum Language {
	case English;
	case Cymru;
	case Gàidhlig;
}
$lang = Language::Gàidhlig;

enum Currency: string {
	case EUR = "€";
	case GBP = "£";
}
$eur = Currency::EUR;

enum Unit: int {
	case Second = 1;
	case Minute = 60;
	case Hour = 3600;
	case Day = 86400;
}
$time = Unit::Hour;

xdebug_var_dump( $lang, $eur, $time );
echo "\n";

xdebug_stop_trace();
?>
--EXPECTF--
%strace_enum-002.php:25:
enum Language::Gàidhlig;
%strace_enum-002.php:25:
enum Currency::EUR : string("€");
%strace_enum-002.php:25:
enum Unit::Hour : int(3600);

Version: %s
File format: 4
TRACE START [%d-%d-%d %d:%d:%d.%d]
2		A						%s	%d	$tf = '%s'
2	%d	1	%f	%d
1		A						%strace_enum-002.php	9	$lang = enum Language::Gàidhlig
1		A						%strace_enum-002.php	15	$eur = enum Currency::EUR('€')
1		A						%strace_enum-002.php	23	$time = enum Unit::Hour(3600)
2	%d	0	%f	%d	xdebug_var_dump	0		%strace_enum-002.php	25	3	enum Language::Gàidhlig	enum Currency::EUR('€')	enum Unit::Hour(3600)
2	%d	1	%f	%d
2	%d	0	%f	%d	xdebug_stop_trace	0		%strace_enum-002.php	28	0
			%f	%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
