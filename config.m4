dnl config.m4 for extension Xdebug

PHP_ARG_ENABLE(xdebug, whether to enable Xdebug support,
[  --enable-xdebug         Enable Xdebug support])

PHP_ARG_ENABLE(xdebug-ssl, [whether to enable SSL support (needed for Xdebug Cloud)],
[  --enable-xdebug-ssl       Xdebug: Enable SSL support], yes, no)

PHP_ARG_ENABLE(xdebug-dev, whether to enable Xdebug developer build flags,
[  --enable-xdebug-dev       Xdebug: Enable developer flags], no, no)

PHP_ARG_WITH(xdebug-compression, [whether to compress profiler files (requires zlib)],
[  --without-xdebug-compression     Xdebug: Disable compression through zlib],yes,no)

m4_include([build/pkg.m4])
m4_include([build/clocks.m4])

if test "$PHP_XDEBUG" != "no"; then
  AC_MSG_CHECKING([Check for supported PHP versions])
  PHP_XDEBUG_FOUND_VERSION=`${PHP_CONFIG} --version`
  PHP_XDEBUG_FOUND_VERNUM=`echo "${PHP_XDEBUG_FOUND_VERSION}" | $AWK 'BEGIN { FS = "."; } { printf "%d", ([$]1 * 100 + [$]2) * 100 + [$]3;}'`
  if test "$PHP_XDEBUG_FOUND_VERNUM" -lt "70300"; then
    AC_MSG_ERROR([not supported. Need a PHP version >= 7.3.0 and < 8.3.0 (found $PHP_XDEBUG_FOUND_VERSION)])
  else
    if test "$PHP_XDEBUG_FOUND_VERNUM" -ge "80300"; then
      AC_MSG_ERROR([not supported. Need a PHP version >= 7.3.0 and < 8.3.0 (found $PHP_XDEBUG_FOUND_VERSION)])
    else
      AC_MSG_RESULT([supported ($PHP_XDEBUG_FOUND_VERSION)])
    fi
  fi

  AC_DEFINE(HAVE_XDEBUG,1,[ ])

  old_CPPFLAGS=$CPPFLAGS
  CPPFLAGS="$INCLUDES $CPPFLAGS"

  AC_XDEBUG_CLOCK

  AC_CHECK_HEADERS([netinet/in.h poll.h sys/poll.h])

  PHP_CHECK_LIBRARY(m, cos, [ PHP_ADD_LIBRARY(m,, XDEBUG_SHARED_LIBADD) ])

  if test "$PHP_XDEBUG_COMPRESSION" != "no"; then
    PKG_CHECK_MODULES([ZLIB], [zlib >= 1.2.9],[
      PHP_EVAL_LIBLINE($ZLIB_LIBS, XDEBUG_SHARED_LIBADD)
      PHP_EVAL_INCLINE($ZLIB_CFLAGS)

      AC_DEFINE(HAVE_XDEBUG_ZLIB,1,[ ])
    ],[ ])
  fi

  CPPFLAGS=$old_CPPFLAGS

  if test "$PHP_XDEBUG_DEV" = "yes"; then
    AX_CHECK_COMPILE_FLAG(-Wbool-conversion,                _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wbool-conversion")
    AX_CHECK_COMPILE_FLAG(-Wdeclaration-after-statement,    _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wdeclaration-after-statement")
    AX_CHECK_COMPILE_FLAG(-Wdiscarded-qualifiers,           _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wdiscarded-qualifiers")
    AX_CHECK_COMPILE_FLAG(-Wduplicate-enum,                 _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wduplicate-enum")
    AX_CHECK_COMPILE_FLAG(-Wempty-body,                     _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wempty-body")
    AX_CHECK_COMPILE_FLAG(-Wenum-compare,                   _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wenum-compare")
    AX_CHECK_COMPILE_FLAG(-Werror,                          _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Werror")
    AX_CHECK_COMPILE_FLAG(-Wextra,                          _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wextra")
    AX_CHECK_COMPILE_FLAG(-Wformat-nonliteral,              _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wformat-nonliteral")
    AX_CHECK_COMPILE_FLAG(-Wformat-security,                _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wformat-security")
    AX_CHECK_COMPILE_FLAG(-Wheader-guard,                   _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wheader-guard")
    AX_CHECK_COMPILE_FLAG(-Wincompatible-pointer-types-discards-qualifiers, _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wincompatible-pointer-types-discards-qualifiers")
    AX_CHECK_COMPILE_FLAG(-Wimplicit-fallthrough,           _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wimplicit-fallthrough")
    AX_CHECK_COMPILE_FLAG(-Winit-self,                      _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Winit-self")
    AX_CHECK_COMPILE_FLAG(-Wlogical-not-parentheses,        _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wlogical-not-parentheses")
    AX_CHECK_COMPILE_FLAG(-Wlogical-op,                     _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wlogical-op")
    AX_CHECK_COMPILE_FLAG(-Wlogical-op-parentheses,         _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wlogical-op-parentheses")
    AX_CHECK_COMPILE_FLAG(-Wloop-analysis,                  _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wloop-analysis")
    AX_CHECK_COMPILE_FLAG(-Wmaybe-uninitialized,            _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wmaybe-uninitialized")
    AX_CHECK_COMPILE_FLAG(-Wmissing-format-attribute,       _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wmissing-format-attribute")
    AX_CHECK_COMPILE_FLAG(-Wno-missing-field-initializers,  _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wno-missing-field-initializers")
    AX_CHECK_COMPILE_FLAG(-Wno-sign-compare,                _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wno-sign-compare")
    AX_CHECK_COMPILE_FLAG(-Wno-unused-but-set-variable,     _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wno-unused-but-set-variable")
    AX_CHECK_COMPILE_FLAG(-Wno-unused-parameter,            _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wno-unused-parameter")
    AX_CHECK_COMPILE_FLAG(-Wno-variadic-macros,             _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wno-variadic-macros")
    AX_CHECK_COMPILE_FLAG(-Wparentheses,                    _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wparentheses")
    AX_CHECK_COMPILE_FLAG(-Wpointer-bool-conversion,        _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wpointer-bool-conversion")
    AX_CHECK_COMPILE_FLAG(-Wsizeof-array-argument,          _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wsizeof-array-argument")
    AX_CHECK_COMPILE_FLAG(-Wstring-conversion,              _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wstring-conversion")
    AX_CHECK_COMPILE_FLAG(-Wwrite-strings,                  _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wwrite-strings")
    AX_CHECK_COMPILE_FLAG(-Wpointer-arith,                  _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wpointer-arith")
    AX_CHECK_COMPILE_FLAG(-fdiagnostics-show-option,        _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -fdiagnostics-show-option")
    AX_CHECK_COMPILE_FLAG(-fno-exceptions,                  _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -fno-exceptions")
    AX_CHECK_COMPILE_FLAG(-fno-omit-frame-pointer,          _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -fno-omit-frame-pointer")
    AX_CHECK_COMPILE_FLAG(-fno-optimize-sibling-calls,      _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -fno-optimize-sibling-calls")
    AX_CHECK_COMPILE_FLAG(-fsanitize-address,               _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -fsanitize-address")
    AX_CHECK_COMPILE_FLAG(-fstack-protector,                _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -fstack-protector")

    MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS"
    STD_CFLAGS="-g -O0 -Wall"
  fi

  PHP_XDEBUG_CFLAGS="$STD_CFLAGS $MAINTAINER_CFLAGS"

  XDEBUG_BASE_SOURCES="src/base/base.c src/base/filter.c"
  XDEBUG_LIB_SOURCES="src/lib/usefulstuff.c src/lib/compat.c src/lib/crc32.c src/lib/file.c src/lib/hash.c src/lib/headers.c src/lib/lib.c src/lib/llist.c src/lib/log.c src/lib/set.c src/lib/str.c src/lib/timing.c src/lib/var.c src/lib/var_export_html.c src/lib/var_export_line.c src/lib/var_export_text.c src/lib/var_export_xml.c src/lib/xml.c"

  XDEBUG_COVERAGE_SOURCES="src/coverage/branch_info.c src/coverage/code_coverage.c"
  XDEBUG_DEBUGGER_SOURCES="src/debugger/com.c src/debugger/debugger.c src/debugger/handler_dbgp.c src/debugger/handlers.c"
  XDEBUG_DEVELOP_SOURCES="src/develop/develop.c src/develop/monitor.c src/develop/php_functions.c src/develop/stack.c src/develop/superglobals.c"
  XDEBUG_GCSTATS_SOURCES="src/gcstats/gc_stats.c"
  XDEBUG_PROFILER_SOURCES="src/profiler/profiler.c"
  XDEBUG_TRACING_SOURCES="src/tracing/trace_computerized.c src/tracing/trace_html.c src/tracing/trace_textual.c src/tracing/tracing.c"

  if test "$PHP_XDEBUG_SSL" = "yes"; then
    dnl Import BearSSL
    BEARSSL_AEAD_SOURCES="lib/bearssl/src/aead/ccm.c"
    BEARSSL_CODEC_SOURCES="lib/bearssl/src/codec/ccopy.c lib/bearssl/src/codec/dec32be.c lib/bearssl/src/codec/dec32le.c lib/bearssl/src/codec/dec64be.c lib/bearssl/src/codec/enc32be.c lib/bearssl/src/codec/enc32le.c lib/bearssl/src/codec/enc64be.c"
    BEARSSL_EC_SOURCES="lib/bearssl/src/ec/ec_all_m31.c lib/bearssl/src/ec/ec_c25519_m31.c lib/bearssl/src/ec/ec_c25519_m64.c lib/bearssl/src/ec/ec_p256_m31.c lib/bearssl/src/ec/ec_p256_m64.c lib/bearssl/src/ec/ec_prime_i31.c lib/bearssl/src/ec/ec_secp256r1.c lib/bearssl/src/ec/ec_secp384r1.c lib/bearssl/src/ec/ec_secp521r1.c lib/bearssl/src/ec/ecdsa_atr.c lib/bearssl/src/ec/ecdsa_i31_bits.c lib/bearssl/src/ec/ecdsa_i31_vrfy_asn1.c lib/bearssl/src/ec/ecdsa_i31_vrfy_raw.c"
    BEARSSL_HASH_SOURCES="lib/bearssl/src/hash/multihash.c lib/bearssl/src/hash/sha2small.c"
    BEARSSL_INT_SOURCES="lib/bearssl/src/int/i31_add.c lib/bearssl/src/int/i31_bitlen.c lib/bearssl/src/int/i31_decmod.c lib/bearssl/src/int/i31_decode.c lib/bearssl/src/int/i31_encode.c lib/bearssl/src/int/i31_fmont.c lib/bearssl/src/int/i31_iszero.c lib/bearssl/src/int/i31_modpow.c lib/bearssl/src/int/i31_modpow2.c lib/bearssl/src/int/i31_montmul.c lib/bearssl/src/int/i31_muladd.c lib/bearssl/src/int/i31_ninv31.c lib/bearssl/src/int/i31_rshift.c lib/bearssl/src/int/i31_sub.c lib/bearssl/src/int/i31_tmont.c lib/bearssl/src/int/i32_div32.c lib/bearssl/src/int/i62_modpow2.c"
    BEARSSL_MAC_SOURCES="lib/bearssl/src/mac/hmac.c lib/bearssl/src/mac/hmac_ct.c"
    BEARSSL_RAND_SOURCES="lib/bearssl/src/rand/hmac_drbg.c lib/bearssl/src/rand/sysrng.c"
    BEARSSL_RSA_SOURCES="lib/bearssl/src/rsa/rsa_default_pkcs1_vrfy.c lib/bearssl/src/rsa/rsa_default_pub.c lib/bearssl/src/rsa/rsa_i31_pkcs1_vrfy.c lib/bearssl/src/rsa/rsa_i62_pkcs1_vrfy.c lib/bearssl/src/rsa/rsa_i31_pub.c lib/bearssl/src/rsa/rsa_i62_pub.c lib/bearssl/src/rsa/rsa_pkcs1_sig_unpad.c"
    BEARSSL_SSL_SOURCES="lib/bearssl/src/ssl/prf.c lib/bearssl/src/ssl/prf_sha256.c lib/bearssl/src/ssl/ssl_client.c lib/bearssl/src/ssl/ssl_client_default_rsapub.c lib/bearssl/src/ssl/ssl_client_full.c lib/bearssl/src/ssl/ssl_engine.c lib/bearssl/src/ssl/ssl_engine_default_chapol.c lib/bearssl/src/ssl/ssl_engine_default_ecdsa.c lib/bearssl/src/ssl/ssl_engine_default_rsavrfy.c lib/bearssl/src/ssl/ssl_hs_client.c lib/bearssl/src/ssl/ssl_io.c lib/bearssl/src/ssl/ssl_rec_chapol.c"
    BEARSSL_SYMCIPHER_SOURCES="lib/bearssl/src/symcipher/chacha20_ct.c lib/bearssl/src/symcipher/chacha20_sse2.c lib/bearssl/src/symcipher/poly1305_ctmul.c lib/bearssl/src/symcipher/poly1305_ctmulq.c"
    BEARSSL_X509_SOURCES="lib/bearssl/src/x509/x509_minimal.c"
    BEARSSL_SOURCES="$BEARSSL_AEAD_SOURCES $BEARSSL_CODEC_SOURCES $BEARSSL_EC_SOURCES $BEARSSL_HASH_SOURCES $BEARSSL_INT_SOURCES $BEARSSL_MAC_SOURCES $BEARSSL_RAND_SOURCES $BEARSSL_RSA_SOURCES $BEARSSL_SSL_SOURCES $BEARSSL_SYMCIPHER_SOURCES $BEARSSL_X509_SOURCES"
  fi

  PHP_NEW_EXTENSION(xdebug, xdebug.c $XDEBUG_BASE_SOURCES $XDEBUG_LIB_SOURCES $XDEBUG_COVERAGE_SOURCES $XDEBUG_DEBUGGER_SOURCES $XDEBUG_DEVELOP_SOURCES $XDEBUG_GCSTATS_SOURCES $XDEBUG_PROFILER_SOURCES $XDEBUG_TRACING_SOURCES $BEARSSL_SOURCES, $ext_shared,,$PHP_XDEBUG_CFLAGS,,yes)

  PHP_ADD_BUILD_DIR(PHP_EXT_BUILDDIR(xdebug)[/src/base])
  PHP_ADD_BUILD_DIR(PHP_EXT_BUILDDIR(xdebug)[/src/lib])
  PHP_ADD_BUILD_DIR(PHP_EXT_BUILDDIR(xdebug)[/src/coverage])
  PHP_ADD_BUILD_DIR(PHP_EXT_BUILDDIR(xdebug)[/src/debugger])
  PHP_ADD_BUILD_DIR(PHP_EXT_BUILDDIR(xdebug)[/src/develop])
  PHP_ADD_BUILD_DIR(PHP_EXT_BUILDDIR(xdebug)[/src/gcstats])
  PHP_ADD_BUILD_DIR(PHP_EXT_BUILDDIR(xdebug)[/src/profiler])
  PHP_ADD_BUILD_DIR(PHP_EXT_BUILDDIR(xdebug)[/src/tracing])

  if test "$PHP_XDEBUG_SSL" = "yes"; then
    PHP_ADD_BUILD_DIR(PHP_EXT_BUILDDIR(xdebug)[/lib/bearssl])
    PHP_ADD_BUILD_DIR(PHP_EXT_BUILDDIR(xdebug)[/lib/bearssl/src])
    PHP_ADD_BUILD_DIR(PHP_EXT_BUILDDIR(xdebug)[/lib/bearssl/src/aead])
    PHP_ADD_BUILD_DIR(PHP_EXT_BUILDDIR(xdebug)[/lib/bearssl/src/codec])
    PHP_ADD_BUILD_DIR(PHP_EXT_BUILDDIR(xdebug)[/lib/bearssl/src/ec])
    PHP_ADD_BUILD_DIR(PHP_EXT_BUILDDIR(xdebug)[/lib/bearssl/src/hash])
    PHP_ADD_BUILD_DIR(PHP_EXT_BUILDDIR(xdebug)[/lib/bearssl/src/int])
    PHP_ADD_BUILD_DIR(PHP_EXT_BUILDDIR(xdebug)[/lib/bearssl/src/mac])
    PHP_ADD_BUILD_DIR(PHP_EXT_BUILDDIR(xdebug)[/lib/bearssl/src/rand])
    PHP_ADD_BUILD_DIR(PHP_EXT_BUILDDIR(xdebug)[/lib/bearssl/src/rsa])
    PHP_ADD_BUILD_DIR(PHP_EXT_BUILDDIR(xdebug)[/lib/bearssl/src/ssl])
    PHP_ADD_BUILD_DIR(PHP_EXT_BUILDDIR(xdebug)[/lib/bearssl/src/symcipher])
    PHP_ADD_BUILD_DIR(PHP_EXT_BUILDDIR(xdebug)[/lib/bearssl/src/x509])
  fi

  PHP_SUBST(XDEBUG_SHARED_LIBADD)
  PHP_ADD_MAKEFILE_FRAGMENT

  PHP_ADD_INCLUDE($ext_srcdir/src)
  PHP_ADD_INCLUDE($ext_builddir/src)
  if test "$PHP_XDEBUG_SSL" = "yes"; then
    PHP_ADD_INCLUDE($ext_srcdir/lib/bearssl/inc)
    PHP_ADD_INCLUDE($ext_builddir/lib/bearssl/inc)
    PHP_ADD_INCLUDE($ext_srcdir/lib/bearssl/src)
    PHP_ADD_INCLUDE($ext_builddir/lib/bearssl/src)

    AC_DEFINE(HAVE_XDEBUG_SSL_SUPPORT, 1, [ ])
  fi
fi
