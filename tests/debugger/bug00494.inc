<?php
class abc {
        private $arr;
        public function __construct(){
                $this->arr = array(
                        0 => array("some", "values"),
                        1 => array("some", "more", "values")
                );
        }
}

class def extends abc {
        private $arr;
}

$o = new def;
echo "o: ";
var_dump($o);
