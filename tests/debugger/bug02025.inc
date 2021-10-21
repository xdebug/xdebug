<?php

eval('
class foo {
  protected $foo = "bar";
  public function bar() {
    xdebug_break();
    echo $this->foo;
  }
}
');

$tmp = new class() extends foo {
  function baz() {
    // step into this.
    $this->bar();
  }
};
$tmp->baz();
