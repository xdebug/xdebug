<?php

function getByFilename()
{
    if (true) {
        if (
            false
        ) {
            $this_file_format = $file_format; // will be shown as not covered
            exit;   // will be shown as not covered
            die;    // will be shown as not covered
            throw new Exception(); // will be shown as covered because it's the last line in the block
        }
    } else {    // the else is crucial even when it's not actually called. and when it's empty. When removed, the coverage bug stops happening.
    }
}

function getByFilename2()
{
    if (true) {
        if (
            false
        ) {
            $this_file_format = $file_format; // will be shown as covered because it's the last (=only) line in the block
        }
    } else {    // the else is crucial even when it's not actually called. and when it's empty. When removed, the coverage bug stops happening.
    }
}
