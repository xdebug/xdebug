<?php
class Test {
  public static $staticMember = 5;
}

Test::$staticMember = 10;

echo Test::$staticMember;

?>
