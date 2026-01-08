<?php

/**
 * Generate performance benchmark summary from callgrind results
 *
 * This script reads benchmark results from the results directory, merges them
 * into a single directory, and generates a markdown summary with performance
 * metrics for different PHP versions, commands, and Xdebug modes.
 */

// Create merged directory and copy all result files into it
if (!is_dir('merged')) {
    mkdir('merged', 0755, true);
    echo "Created merged directory\n";
}

// Check if results directory exists
if (!is_dir('results')) {
    fwrite(STDERR, "Error: results directory not found\n");
    exit(1);
}

// Recursively find all .txt files in results directory and copy them to merged
$iterator = new RecursiveIteratorIterator(
    new RecursiveDirectoryIterator('results', RecursiveDirectoryIterator::SKIP_DOTS),
    RecursiveIteratorIterator::LEAVES_ONLY
);

$fileCount = 0;
foreach ($iterator as $file) {
    if ($file->isFile() && $file->getExtension() === 'txt') {
        copy($file->getPathname(), 'merged/' . $file->getFilename());
        $fileCount++;
    }
}
echo "Copied $fileCount files to merged directory\n";

// Now merge all matrix-values files into a single unique list
$matrixFiles = glob('merged/matrix-values-*.txt');
if ($matrixFiles === false || $matrixFiles === []) {
    fwrite(STDERR, "Error: No matrix-values files found in merged directory\n");
    exit(1);
}

$allMatrixValues = [];
foreach ($matrixFiles as $file) {
    $lines = file($file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
    if ($lines !== false) {
        $allMatrixValues = array_merge($allMatrixValues, $lines);
    }
}

// Get unique values and sort them
$allMatrixValues = array_unique($allMatrixValues);
sort($allMatrixValues);

// Write the unique matrix values to file
file_put_contents('unique-matrix-values.txt', implode("\n", $allMatrixValues) . "\n");
echo "Created unique-matrix-values.txt with " . count($allMatrixValues) . " unique combinations\n";

// Parse unique matrix values to get all combinations
$matrixValues = $allMatrixValues;

// Extract unique values for each dimension
$commands = [];
$phpVersions = [];
$xdebugModes = [];

foreach ($matrixValues as $line) {
    [$php, $command, $xdebug] = explode(',', $line);
    $phpVersions[$php] = true;
    $commands[$command] = true;
    $xdebugModes[$xdebug] = true;
}

// Sort the arrays
$commands = array_keys($commands);
$phpVersions = array_keys($phpVersions);
$xdebugModes = array_keys($xdebugModes);
sort($commands);
sort($phpVersions);
sort($xdebugModes);

// Start building the markdown summary and CSV data
$output = "# 🕒 Performance Results\n";
$csvData = [];
$csvData[] = ['command', 'php', 'xdebug_mode', 'instructions', 'slowdown'];

// Loop through each command
foreach ($commands as $command) {
    $output .= "\n## **Command:** `$command`\n";

    // Loop through each PHP version
    foreach ($phpVersions as $php) {
        $output .= "\n### **PHP Version:** `$php`\n\n";
        $output .= "| Xdebug | Instructions | Slowdown |\n";
        $output .= "|--------|-------------:|---------:|\n";

        // Get base value (when Xdebug mode is "no")
        $baseFile = "merged/php-{$php}_cmd-{$command}_xdebug-no.txt";
        if (!file_exists($baseFile)) {
            fwrite(STDERR, "Warning: Base file not found: $baseFile\n");
            continue;
        }

        $baseValue = (int)trim(file_get_contents($baseFile));

        // Loop through each Xdebug mode
        foreach ($xdebugModes as $xdebug) {
            $file = "merged/php-{$php}_cmd-{$command}_xdebug-{$xdebug}.txt";

            if (!file_exists($file)) {
                continue;
            }

            $value = (int)trim(file_get_contents($file));

            // Calculate slowdown
            if ($xdebug === 'no') {
                $slowdown = '0%';
                $slowdownPercent = 0.0;
            } else {
                $slowdownPercent = (($value - $baseValue) * 100) / $baseValue;
                $slowdown = sprintf('%.1f%%', $slowdownPercent);
            }

            // Format the value with thousands separators for markdown
            $formattedValue = number_format($value);

            $output .= "| $xdebug | $formattedValue | $slowdown |\n";

            // Add to CSV data (with raw numbers, not formatted)
            $csvData[] = [$command, $php, $xdebug, $value, sprintf('%.1f', $slowdownPercent)];
        }
    }
}

// Write the summary to file
file_put_contents('summary.md', $output);
echo "Summary generated successfully in summary.md\n";

// Write CSV file with metadata header
$csvFile = fopen('summary.csv', 'w');

// Add metadata rows at the top
fputcsv($csvFile, ['date', date('c')]); // ISO 8601 format

// For pull requests, GITHUB_HEAD_REF contains the source branch name
// For other events (workflow_dispatch, schedule), use GITHUB_REF_NAME which contains the current branch name
$branch = getenv('GITHUB_HEAD_REF') ?: getenv('GITHUB_REF_NAME') ?: 'unknown';
fputcsv($csvFile, ['branch', $branch]);

fputcsv($csvFile, ['commit', getenv('GITHUB_SHA') ?: 'unknown']);

// Add the benchmark data
foreach ($csvData as $row) {
    fputcsv($csvFile, $row);
}
fclose($csvFile);
echo "CSV data generated successfully in summary.csv\n";
