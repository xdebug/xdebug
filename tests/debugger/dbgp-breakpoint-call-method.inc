<?php
class breaking
{
	function onMe()
	{
		echo "break in the onMe method\n";
	}

	static function staticMe()
	{
		echo "break in the staticMe method\n";
	}
}

breaking::staticMe();

$a = new breaking;
$a->onMe();
$a->staticMe();
$a->onMe();
?>
