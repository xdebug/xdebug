<?php
require getenv("XDEBUG_TEST_DIR") . '/bug00627-symlink.inc';

function normal()
{
	inSymlinkedFile();
}

normal();
?>
