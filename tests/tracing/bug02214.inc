<?php
function func(array $a) {
    return $a;
}

func([
    "\n" => "\n",
    "\r" => "\r",
    "\r\n" => "\r\n",
]);
?>
