<?php
$connection = new MongoClient();
$collection = $connection->phpunit->test;
$collection->drop();

$collection->batchInsert(array(
    array( 'a' => 'b'),
    array( 'a' => 'b'),
));

$cursor = $collection->find();
$collection->drop();
?>
