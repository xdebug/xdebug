<?php
function getCurrentDomain(){
    static $domain;
    if (!isset($domain)) {
        $domain = implode('.', array_slice(explode('.', $_SERVER['SERVER_NAME']), -2));}
    
    return $domain;}
?>
