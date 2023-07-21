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

echo "Hi!\n";
?>
