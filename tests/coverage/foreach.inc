<?php
function showLanguages(array|DatePeriod $languages): void
{
	foreach ($languages as $language) {
		echo $language, "\n";
	}
}
?>
