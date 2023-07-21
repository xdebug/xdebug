<?php

try
{
    throw new Exception();
	echo strlen( "Revenge is a dish best served cold.\n" );
}
catch(Exception $e)
{
}

echo strlen( "The fire is always hotter on someone elses face." ), "\n";
?>
