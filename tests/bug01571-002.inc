<?php
namespace Testing;

$cb = function($a) { usleep(500); return $a * 3; };
$cb(4);
?>
