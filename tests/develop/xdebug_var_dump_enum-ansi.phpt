--TEST--
xdebug_var_dump() with enums [ansi]
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
xdebug.cli_color=2
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
[1m%sxdebug_var_dump_enum-ansi.php[22m:[1m23[22m:
[1menum[22m [31mLanguage[0m::[31mGàidhlig[0m;
[1m%sxdebug_var_dump_enum-ansi.php[22m:[1m23[22m:
[1menum[22m [31mCurrency[0m::[31mEUR[0m : [1mstring[0m([31m"€"[0m);
[1m%sxdebug_var_dump_enum-ansi.php[22m:[1m23[22m:
[1menum[22m [31mUnit[0m::[31mHour[0m : [1mint[0m([32m3600[0m);
