--TEST--
Test for bug #2041: __debugInfo is not used for var_dump output
--FILE--
<?php
final class X
{
    private $secretValue;

    public function __construct(string $secretValue)
    {
        $this->secretValue = $secretValue;
    }

    public function __debugInfo()
    {
        $properties = get_object_vars($this);
        $properties['secretValue'] = empty($properties['secretValue']) ? '' : '********';
        return $properties;
    }
}

$a = new X('supersecret');
var_dump($a);
?>
--EXPECTF--
object(X)#1 (%d) {
  ["secretValue"]=>
  string(8) "********"
}
