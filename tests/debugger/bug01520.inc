<?php
$obj = new stdClass;

$dash = "with-dash-char";
$obj->$dash = 42;

$square = "with['square']";
$obj->$square = 43;

$brace = "{with";
$obj->$brace = 45;

$close = "{wi\"th";
$obj->$close = 46;

$square2 = "two['square']";
$obj->$square2 = [ $close => 4, $brace => 5 ];

echo $name, "\n";
?>
