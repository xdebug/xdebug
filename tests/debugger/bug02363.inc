<?php
$myString = "Hello World";
$result = '<' . $myString . '>'


    |> \htmlentities(...)


    |>
		\str_split(...)

    |> (fn($x) => array_map(strtoupper(...), $x))

    |> (fn($x) => join( ', ', $x));

echo $result, "\n";
