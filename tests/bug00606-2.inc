<?php
error_reporting(-1);
function test()
{
	echo base64_encode("testing"), "\n";
	strlen();
}
test();
echo str_replace("x", "Done", "x with testing"), "\n";
echo "DONE\n";
?>
