<?php
function iterateXML()
{
    $siteXMLString = file_get_contents( FILE, true);
    $iterator = new SimpleXMLIterator($siteXMLString);
    $projectsIterator = $iterator->xpath("//page");

    // Iterate through all 3 occurances of <page> elements
    foreach ($projectsIterator as $pageXML)
    {
        // Set a breakpoint somewhere inside this loop.
        // You won't see any variables in the Local Variables panel until the second
        // time through the loop. After that, they seem to work fine.
        $currentPageXML= $pageXML;
        $name = $currentPageXML["name"];
        echo $name . '
';
    }
}

iterateXML();

