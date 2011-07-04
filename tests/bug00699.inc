<?php
class SomeClass implements Countable {
    private $data = array();

    public function addData($var) {
        $this->data[] = $var;
    }   








    public function count() {
        return count($this->data);
    }   
}
?>
