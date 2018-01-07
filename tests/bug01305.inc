<?php

$settings = array();

//This doesn't work
$settings["One's Stuff"]['id'] = 1000;
$settings["Two\"s Stuff"]['id'] = 2000;
$settings["Three\\s Stuff"]['id'] = 3000;
$settings["Four'sStuff"]['id'] = 4000;
$settings["Five'sSt\0ff"]['id'] = 5000;
echo "hello!\n";

?>
