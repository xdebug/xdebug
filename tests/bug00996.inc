<?php

class Clazz extends ArrayObject{

}

$a = new Clazz();
$a->b = array(1,2,'c' => 3);

echo $a->b[0];

$a['f'] = 'cookies';

echo $a['f'];

?>
