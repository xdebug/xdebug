<?php
$object1 = new stdClass();
$object1->someProperty = "o1";

$object2 = new stdClass();
$object2->someProperty = "o2";

$storage = new SplObjectStorage();
$storage->attach($object1);
$storage->attach($object2);

var_dump($storage); //breakpoint here
?>
