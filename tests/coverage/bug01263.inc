<?php
/*
 * This file is part of the PHP_CodeCoverage package.
 *
 * (c) Sebastian Bergmann <sebastian@phpunit.de>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

class PHP_CodeCoverage
{
    private function addUncoveredFilesFromWhitelist()
    {
        foreach ($uncoveredFiles as $uncoveredFile) {
            for ($i = 1; $i <= $lines; $i++) {
                $data[$uncoveredFile][$i] = PHP_CodeCoverage_Driver::LINE_NOT_EXECUTED;
            }
        }
    }
}
