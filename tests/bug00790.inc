<?php
function callback($buffer)
{
	  // replace all the apples with oranges
	  return "[$buffer]\n";
}

function foo()
{
	// empty func
	echo "again\n";
}
xdebug_break();
trigger_error('test');
echo "test\n";
foo();
?>
