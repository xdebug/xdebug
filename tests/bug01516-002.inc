<?php
$obj = new stdClass;
$name = "with-\0-null-char";
$obj->$name = 42;

$$name = 42;

echo "We're here\n";
?>
