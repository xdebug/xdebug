<?php



class Foo
{
    public function sort($array)
    {
        usort(
            $array,
            function ($x, $y) { return true; }
        );

        \usort(
            $array,
            function ($x, $y) { return true; }
        );

        return $array;
    }
}
?>
