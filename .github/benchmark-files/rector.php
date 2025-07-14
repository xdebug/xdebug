<?php

declare(strict_types=1);

use Rector\Config\RectorConfig;

return RectorConfig::configure()
    ->withPreparedSets(
        deadCode: true,
        codeQuality: true,
        codingStyle: true,
        typeDeclarations: true,
        privatization: true,
        strictBooleans: true,
        instanceOf: true,
        earlyReturn: true,
        naming: true,
        rectorPreset: true,
        phpunitCodeQuality: true
    )
    ->withPaths([
        __DIR__ . '/vendor/symfony/http-foundation',
    ])
;
