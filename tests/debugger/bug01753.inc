<?php
require_once(__DIR__ . '/bug01753-SkeletonPage.inc');
require_once(__DIR__ . '/bug01753-PageIndex.inc');
$page = new PageIndex();
$page->getData();
