<?php
class ExampleTest
{
    public function arrayBuild()
    {
        $arr = [];
        // Append to array.
        $arr[] = [
            'num' => 0,
        ];
        for ($i = 1; $i < 3; $i++) {
            $arr[] = [
                'num' => $i,
            ];
        }
        // Explicit index.
        list($a, $c, $e) = ['b', 'd', 'f'];
        $arr['string-index'] = [
            'num' => 42,
            'sub' => [
                'a' => $a,
                'c' => $c,
                'e' => $e,
            ],
        ];
        // Multi-line assignment
        $arr['multiline-scalar-1']
            = $arr['multiline-scalar-2']
            = 'scalar';
        return $arr;
    }
}
