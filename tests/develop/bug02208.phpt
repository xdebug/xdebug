--TEST--
Test for bug #2208: Superfluous ... (three omission dots) in var_dump()
--INI--
xdebug.mode=develop
xdebug.var_display_max_depth=3
--FILE--
<?php
$a = array(array(array(array())));

$o = new StdClass;
$o->one = new StdClass;
$o->one->two = new StdClass;
$o->one->two->three = new StdClass;

ini_set('html_errors', 0);
var_dump($a, $o);

ini_set('html_errors', 1);
var_dump($a, $o);
?>
--EXPECTF--
%s:%d:
array(1) {
  [0] =>
  array(1) {
    [0] =>
    array(1) {
      [0] =>
      array(0) {
      }
    }
  }
}
%s:%d:
class stdClass#1 (1) {
  public $one =>
  class stdClass#2 (1) {
    public $two =>
    class stdClass#3 (1) {
      public $three =>
      class stdClass#4 (0) {
      }
    }
  }
}
<pre class='xdebug-var-dump' dir='ltr'>
<small>%s:%d:</small>
<b>array</b> <i>(size=1)</i>
  0 <font color='#888a85'>=&gt;</font> 
    <b>array</b> <i>(size=1)</i>
      0 <font color='#888a85'>=&gt;</font> 
        <b>array</b> <i>(size=1)</i>
          0 <font color='#888a85'>=&gt;</font> 
            <b>array</b> <i>(size=0)</i>
</pre><pre class='xdebug-var-dump' dir='ltr'>
<small>%s:%d:</small>
<b>object</b>(<i>stdClass</i>)[<i>1</i>]
  <i>public</i> 'one' <font color='#888a85'>=&gt;</font> 
    <b>object</b>(<i>stdClass</i>)[<i>2</i>]
      <i>public</i> 'two' <font color='#888a85'>=&gt;</font> 
        <b>object</b>(<i>stdClass</i>)[<i>3</i>]
          <i>public</i> 'three' <font color='#888a85'>=&gt;</font> 
            <b>object</b>(<i>stdClass</i>)[<i>4</i>]
</pre>
