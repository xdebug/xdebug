#!/bin/bash

start_time_ms=$(date +%s%3N)

vendor/bin/rector --dry-run --xdebug --debug > /dev/null 2>&1

end_time_ms=$(date +%s%3N)

elapsed_time_ms=$((end_time_ms - start_time_ms))
if [ "$XDEBUG_MODE" = "off" ]; then
  echo "RECTOR_BASE=$elapsed_time_ms" >> $GITHUB_ENV
  slowdown="-"
else
  slowdown="$(((elapsed_time_ms - RECTOR_BASE) * 100 / RECTOR_BASE))%"
fi
elapsed_time_sec=$(printf "%.1f" "$(echo "$elapsed_time_ms / 1000" | bc -l)")
echo "| ${XDEBUG_MODE} | Rector | ${elapsed_time_sec}s | ${slowdown} |" >> summary.md
