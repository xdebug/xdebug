<?php



class Foo
{
    public function sort($array)
    {
        usort(
            $array,
            function ($x, $y) { return 1; }
        );

        \usort(
            $array,
            function ($x, $y) { return 1; }
        );

        return $array;
    }
}
?>
