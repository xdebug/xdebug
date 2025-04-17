#!/bin/bash

start_time_ms=$(date +%s%3N)

vendor/bin/phpstan --memory-limit=-1 --debug --xdebug > /dev/null 2>&1

end_time_ms=$(date +%s%3N)

elapsed_time_ms=$((end_time_ms - start_time_ms))
if [ "$XDEBUG_MODE" = "off" ]; then
  echo "PHPSTAN_BASE=$elapsed_time_ms" >> $GITHUB_ENV
  slowdown="-"
else
  slowdown="$(((elapsed_time_ms - PHPSTAN_BASE) * 100 / PHPSTAN_BASE))%"
fi
elapsed_time_sec=$(printf "%.1f" "$(echo "$elapsed_time_ms / 1000" | bc -l)")
echo "| ${XDEBUG_MODE} | PHPStan | ${elapsed_time_sec}s | ${slowdown} |" >> summary.md
