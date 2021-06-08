--TEST--
Test for enums [1]
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.1');
?>
--INI--
xdebug.mode=trace
xdebug.start_with_request=0
xdebug.collect_return=0
xdebug.collect_assignments=1
xdebug.trace_format=1
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

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
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
%strace_enum-002.php:25:
enum Language::Gàidhlig;
%strace_enum-002.php:25:
enum Currency::EUR : string("€");
%strace_enum-002.php:25:
enum Unit::Hour : int(3600);

Version: 3.1.0-dev
File format: 4
TRACE START [%d-%d-%d %d:%d:%d.%d]
1		A						%strace_enum-002.php	2	$tf = '%s'
1		A						%strace_enum-002.php	9	$lang = enum Language::Gàidhlig
1		A						%strace_enum-002.php	15	$eur = enum Currency::EUR('€')
1		A						%strace_enum-002.php	23	$time = enum Unit::Hour(3600)
2	4	0	%f	%d	xdebug_var_dump	0		%strace_enum-002.php	25	3	enum Language::Gàidhlig	enum Currency::EUR('€')	enum Unit::Hour(3600)
2	4	1	%f	%d
2	5	0	%f	%d	xdebug_stop_trace	0		%strace_enum-002.php	28	0
			%f	%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
