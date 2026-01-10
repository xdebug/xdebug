<?php

class T {
    function __construct() {
        $this->a = [new stdClass()];

        // Put a breakpoint on the echo and open the evaluator and type in the expression:
        // $this->a[0]::class
        echo $this->a[0]::class;
    }
}

new T;
?>
