<?php

$settings = array();

//This doesn't work
$settings["One's Stuff"]['id'] = 1000;
$settings["Two\"s Stuff"]['id'] = 2000;
$settings["Three\\s Stuff"]['id'] = 3000;

echo "hello!\n";

?>
