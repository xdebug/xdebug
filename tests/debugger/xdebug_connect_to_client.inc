<?php
function worker($i)
{
	if ($i == 1) {
		xdebug_connect_to_client();
	}

	echo "$i\n";
}

for ($i = 0; $i < 5; $i++)
{
	worker($i);
}
?>
