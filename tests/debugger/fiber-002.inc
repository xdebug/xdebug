<?php

(new Fiber(function (): void {
    $fiber1 = new Fiber(function (): void {
        $a = 1;
        Fiber::suspend();
        $a = 2;
    });
    $fiber1->start();

    $fiber2 = new Fiber(function (): void {
        $a = 1;
        Fiber::suspend();
        $a = 2;
    });
    $fiber2->start();

    $fiber3 = new Fiber(function (): void {
        $a = 1;
        Fiber::suspend();
        $a = 2;
    });
    $fiber3->start();

    $fiber1->resume();
    $fiber2->resume();
    $fiber3->resume();
    $a = 1;
}))->start();
?>
