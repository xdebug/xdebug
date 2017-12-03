@echo off

set NO_INTERACTION=1
set REPORT_EXIT_STATUS=1
set SKIP_IO_CAPTURE_TESTS=1

if /i "%APPVEYOR_REPO_BRANCH:~0,7%" equ "XDEBUG_" (
    set BRANCH=%APPVEYOR_REPO_BRANCH:~7,3%
    set STABILITY=stable
) else (
    set BRANCH=master
    set STABILITY=staging
)

set BRANCH=%PHP_REL%

set DEPS_DIR=%PHP_BUILD_CACHE_BASE_DIR%\deps-%BRANCH%-%PHP_SDK_VC%-%PHP_SDK_ARCH%
if not exist "%DEPS_DIR%" (
    echo "%DEPS_DIR%" doesn't exist
    exit /b 3
)

rem prepare for Opcache
if "%OPCACHE%" equ "1" set OPCACHE_OPTS=-d opcache.enabled=1 -d opcache.enable_cli=1

mkdir c:\tests_tmp

cd "%APPVEYOR_BUILD_FOLDER%"
nmake test TESTS="%OPCACHE_OPTS% -q --offline --show-diff --show-slow 1000 --set-timeout 120 -g FAIL,XFAIL,BORK,WARN,LEAK,SKIP --temp-source c:\tests_tmp --temp-target c:\tests_tmp"

exit /b %errorlevel%

