# Builds Xdebug from the local checkout against FrankenPHP's bundled ZTS PHP,
# then runs it in worker mode so the SAPI activate/deactivate hooks from
# src/debugger/frankenphp.c can be exercised.
#
# Build (context must be the repository root):
#   docker build -f tests/frankenphp/Dockerfile -t xdebug-frankenphp .
#   docker build --build-arg PHP_VERSION=8.5 -f tests/frankenphp/Dockerfile .
#
# Run (IDE listening on the host, port 9003):
#   docker run --rm -p 8080:8080 xdebug-frankenphp

ARG PHP_VERSION=8.4

FROM dunglas/frankenphp:1-php${PHP_VERSION} AS build

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential autoconf \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . /src

RUN phpize --clean 2>/dev/null || true \
 && phpize \
 && ./configure --enable-xdebug \
 && make -j"$(nproc)" \
 && cp modules/xdebug.so "$(php-config --extension-dir)/"

# ---- runtime ----
FROM dunglas/frankenphp:1-php${PHP_VERSION}

COPY --from=build /usr/local/lib/php/extensions/ /usr/local/lib/php/extensions/

RUN { \
      echo 'zend_extension=xdebug.so'; \
      echo 'xdebug.mode=debug'; \
      echo 'xdebug.start_with_request=trigger'; \
      echo 'xdebug.client_host=xdebug://gateway'; \
      echo 'xdebug.client_port=9003'; \
      echo 'xdebug.discover_client_host=0'; \
      echo 'xdebug.log=/tmp/xdebug.log'; \
      echo 'xdebug.log_level=7'; \
    } > /usr/local/etc/php/conf.d/zz-xdebug.ini

COPY tests/frankenphp/app/       /app/public/
COPY tests/frankenphp/Caddyfile  /etc/frankenphp/Caddyfile
COPY tests/frankenphp/listener.php /usr/local/bin/listener.php
COPY --chmod=755 tests/frankenphp/entrypoint.sh /usr/local/bin/test-entrypoint.sh

ENTRYPOINT ["/usr/local/bin/test-entrypoint.sh"]
CMD ["frankenphp", "run", "--config", "/etc/frankenphp/Caddyfile"]

EXPOSE 8080
