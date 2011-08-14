<?php
function test() {
	echo base64_decode("bmV3IE5vbkV4aXN0c0NsYXNz"), "\n";
	xdebug_break();
}

test();
echo base64_decode("bmV3IE5vbkV4aXN0c0NsYXNz"), "\n";
echo "DONE\n";
?>
