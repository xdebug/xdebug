<?php

xdebug_start_code_coverage(XDEBUG_CC_UNUSED);

$fiber = new \Fiber(function () {
    $fiber = new \Fiber(function () { });
    var_dump($fiber->start() === null);
});
var_dump($fiber->start() === null);
