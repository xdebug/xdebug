<?php
$obj = new stdClass;
$name = "with_\0_null_char";
$obj->$name = 42;

echo "We're here\n";
?>
