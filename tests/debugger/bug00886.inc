<?php
$DIR = getenv("XDEBUG_TEST_DIR");
include "phar://$DIR/bug00886.phar/file1.php";
function1( "Derick" );
?>
