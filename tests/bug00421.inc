<?php
function iterateXML()
{
    $siteXMLString = <<<ENDXML
<?xml version="1.0" encoding="UTF-8"?>
<site>
    <page name="projects">
        <page name="Project 1" />
        <page name="Project 2" />
    </page>
</site>

ENDXML;
    $siteXMLString = str_replace( "\r\n", "\n", $siteXMLString );
    $iterator = new SimpleXMLIterator($siteXMLString);
    $projectsIterator = $iterator->xpath("//page");

    // Iterate through all 3 occurrences of <page> elements
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

