<?php
declare(ticks=1);
class UncoveredLine
{
    private $_foo;

    function __construct($foo)
    {
        if ($foo === 'bar') {
            $this->_foo = $foo;
        } else {
            $this->_foo = null;
        }
    }

    function getFoo()
    {
     return $this->_foo;
    }
}
?>
