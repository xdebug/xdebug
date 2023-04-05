<?php
$dll = new SplDoublyLinkedList();
$dll->push('a');
$dll->push('b');
$dll->push('c');
$dll->push('d');

$priorityQueue = new SplPriorityQueue();
$priorityQueue->insert('A', 3);
$priorityQueue->insert('B', 6);
$priorityQueue->insert('C', 1);
$priorityQueue->insert('D', 2);

print_r($dll, $priorityQueue);
?>
