<?php
$DIR = getenv("XDEBUG_TEST_DIR");
include $DIR . '/bug01007-simpleclass.inc';

echo SimpleClass::init();
?>
