<?php
require_once('bug00643-t2.inc');
 















 
function t1()
{
	global $a;
	if (count($a) && errors_fatal($a))
	{
		echo "ok";
	}
	return TRUE;
}
?>
