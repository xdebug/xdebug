--TEST--
Test for enums [0]
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
xdebug.trace_format=0
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
%strace_enum-001.php:25:
enum Language::Gàidhlig;
%strace_enum-001.php:25:
enum Currency::EUR : string("€");
%strace_enum-001.php:25:
enum Unit::Hour : int(3600);

TRACE START [%d-%d-%d %d:%d:%d.%d]
%w               => $tf = '%s' %s
%w             => $lang = enum Language::Gàidhlig %strace_enum-001.php:9
%w             => $eur = enum Currency::EUR('€') %strace_enum-001.php:15
%w             => $time = enum Unit::Hour(3600) %strace_enum-001.php:23
%w%f %w%d     -> xdebug_var_dump(...$variable = variadic(0 => enum Language::Gàidhlig, 1 => enum Currency::EUR('€'), 2 => enum Unit::Hour(3600))) %strace_enum-001.php:25
%w%f %w%d     -> xdebug_stop_trace() %strace_enum-001.php:28
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
