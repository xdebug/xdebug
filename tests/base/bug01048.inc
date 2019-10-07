<?php

function foo() {
    $GLOBALS['cache'] = "cache";
    xdebug_break(); // 1
    echo 1;
}

foo();
xdebug_break(); // 2
echo 1;
?>
