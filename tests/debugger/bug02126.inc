<?php
$globalVar = 42;

function dateStuff()
{
	$tz = new \DateTimeZone('Europe/Amsterdam');
	$ts = $tz->getTransitions(1593186495, 1711813695);
	$ts = $tz->getTransitions(strtotime("2020-01-01"));

	var_dump($ts);
}

dateStuff();
