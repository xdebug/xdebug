<?php

declare(strict_types=1);

use Rector\Config\RectorConfig;
use Rector\TypeDeclaration\Rector\StmtsAwareInterface\DeclareStrictTypesRector;

return RectorConfig::configure()
    ->withPreparedSets(
        rectorPreset: true,
    )
    ->withPaths([
        __DIR__ . '/.github/benchmark-files/rector.php',
    ])
;
