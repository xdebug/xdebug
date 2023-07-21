--TEST--
xdebug_var_dump() with enums [html]
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.1');
?>
--INI--
date.timezone=UTC
xdebug.mode=develop
html_errors=1
xdebug.var_display_max_children=11
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
<pre class='xdebug-var-dump' dir='ltr'>
<small>%sxdebug_var_dump_enum-html.php:23:</small><b>enum</b>(<i>Language::Gàidhlig</i>)
</pre><pre class='xdebug-var-dump' dir='ltr'>
<small>%sxdebug_var_dump_enum-html.php:23:</small><b>enum</b>(<i>Currency::EUR</i>) : <small>string</small> <font color='#cc0000'>'€'</font> <i>(length=3)</i>
</pre><pre class='xdebug-var-dump' dir='ltr'>
<small>%sxdebug_var_dump_enum-html.php:23:</small><b>enum</b>(<i>Unit::Hour</i>) : <small>int</small> <font color='#4e9a06'>3600</font>
</pre>
