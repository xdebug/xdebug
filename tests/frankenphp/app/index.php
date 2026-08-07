<?php
/* Served in regular (non-worker) mode: sanity check that classic requests
 * still behave with the worker-mode hooks installed. */
echo 'index pid=' . getmypid() . "\n";
