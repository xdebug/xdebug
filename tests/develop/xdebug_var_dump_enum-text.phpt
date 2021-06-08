--TEST--
xdebug_var_dump() with enums [text]
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.1');
?>
--INI--
date.timezone=UTC
xdebug.mode=develop
html_errors=0
xdebug.var_display_max_children=11
xdebug.cli_color=0
xdebug.file_link_format=
xdebug.filename_format=
--FILE--
<?php
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
?>
--EXPECTF--
%sxdebug_var_dump_enum-text.php:23:
enum Language::Gàidhlig;
%sxdebug_var_dump_enum-text.php:23:
enum Currency::EUR : string("€");
%sxdebug_var_dump_enum-text.php:23:
enum Unit::Hour : int(3600);
