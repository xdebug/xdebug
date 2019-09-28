<?php


function phpErrorCatcher($no = 0, $str = null, $file = null, $line = 0) {
    throw new Exception($str);
}

set_error_handler("phpErrorCatcher", E_ALL);

function my_exception_handler() {
    xdebug_break();
}

set_exception_handler("my_exception_handler");



function causeError() {
    echo $undefined;
}



causeError();
?>
