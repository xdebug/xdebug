<?php
require_once('bug00643-t2.inc');
 















 
function t1()
{
	global $a;
	if (array_count_values($a) && errors_fatal($a))
	{
		echo "ok";
	}
	return TRUE;
}
?>
