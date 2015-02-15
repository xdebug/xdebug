<?php
function scope0() {
    $var = 'scope0';
    $break = null;
}

function scope1() {
    $var = 'scope1';
    scope0();
}

function scope2() {
    $var = 'scope2';
    scope1();
}

scope2();

?>
