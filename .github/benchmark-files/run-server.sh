#!/bin/bash

php -S 127.0.0.1:8000 -t public > /dev/null 2>&1 & echo $! > /tmp/php-server.pid

start_time_ms=$(date +%s%3N)

for i in {1..2000}; do
  wget -q -O /dev/null http://127.0.0.1:8000
done

end_time_ms=$(date +%s%3N)

kill $(cat /tmp/php-server.pid)

elapsed_time_ms=$((end_time_ms - start_time_ms))
if [ "$XDEBUG_MODE" = "off" ]; then
  echo "SERVER_BASE=$elapsed_time_ms" >> $GITHUB_ENV
  slowdown="-"
else
  slowdown="$(((elapsed_time_ms - SERVER_BASE) * 100 / SERVER_BASE))%"
fi
elapsed_time_sec=$(printf "%.1f" "$(echo "$elapsed_time_ms / 1000" | bc -l)")
echo "| ${XDEBUG_MODE} | Server | ${elapsed_time_sec}s | ${slowdown} |" >> summary.md
