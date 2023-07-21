--TEST--
Test for bug #2041: __debugInfo is not used for var_dump output
--INI--
xdebug.mode=develop
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
%sbug02041.php:%d:
class X#%d (1) {
  public $secretValue =>
  string(8) "********"
}
