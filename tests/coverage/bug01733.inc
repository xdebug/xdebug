<?php
namespace FancyNamespace;

class ConfigProvider
{
    public function test()
    {
        return array_merge(
            require(__DIR__ . '/bug01733a.inc'),
            require(__DIR__ . '/bug01733a.inc')
        );
    }
}

$c = new ConfigProvider;
var_dump( $c->test() );
?>
