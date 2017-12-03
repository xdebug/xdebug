@echo off
setlocal enableextensions enabledelayedexpansion

	cd /D %APPVEYOR_BUILD_FOLDER%
	if %errorlevel% neq 0 exit /b 3

	set STABILITY=staging
	set DEPS_DIR=%PHP_BUILD_CACHE_BASE_DIR%\deps-%PHP_REL%-%PHP_SDK_VC%-%PHP_SDK_ARCH%
	rem SDK is cached, deps info is cached as well
	echo Updating dependencies in %DEPS_DIR%
	cmd /c phpsdk_deps --update --no-backup --branch %PHP_REL% --stability %STABILITY% --deps %DEPS_DIR%
	if %errorlevel% neq 0 exit /b 3

	rem Something went wrong, most likely when concurrent builds were to fetch deps
	rem updates. It might be, that some locking mechanism is needed.
	if not exist "%DEPS_DIR%" (
		cmd /c phpsdk_deps --update --force --no-backup --branch %PHP_REL% --stability %STABILITY% --deps %DEPS_DIR%
	)
	if %errorlevel% neq 0 exit /b 3

	if "%ZTS_STATE%"=="enable" set ZTS_SHORT=ts
	if "%ZTS_STATE%"=="disable" set ZTS_SHORT=nts
	if "%ZTS_STATE%"=="enable" set ZTS_IN_FILENAME=
	if "%ZTS_STATE%"=="disable" set ZTS_IN_FILENAME=-nts
	if "%PHP_SDK_ARCH%"=="x86" set ARCH_IN_FILENAME=
	if "%PHP_SDK_ARCH%"=="x64" set ARCH_IN_FILENAME=-x86_64

	cd /d C:\projects\php-src

	cmd /c buildconf.bat --force

	if %errorlevel% neq 0 exit /b 3

	cmd /c configure.bat --disable-all --with-mp=auto --enable-cli --%ZTS_STATE%-zts --with-xdebug=shared --enable-object-out-dir=%PHP_BUILD_OBJ_DIR% --with-config-file-scan-dir=%APPVEYOR_BUILD_FOLDER%\build\modules.d --with-prefix=%APPVEYOR_BUILD_FOLDER%\build --with-php-build=%DEPS_DIR%

	if %errorlevel% neq 0 exit /b 3

	nmake /NOLOGO
	if %errorlevel% neq 0 exit /b 3

	nmake install

	if %errorlevel% neq 0 exit /b 3

	mkdir c:\tests_tmp
	set TEST_PHP_EXECUTABLE=%APPVEYOR_BUILD_FOLDER%\build\php.exe
	set TEST_PHP_JUNIT=c:\tests_tmp\tests-junit.xml
	if "%OPCACHE%" equ "1" set TEST_PHP_ARGS=!TEST_PHP_ARGS! -d zend_extension=%APPVEYOR_BUILD_FOLDER%\build\ext\php_opcache.so -d opcache.enable=1 -d opcache.enable_cli=1
	set TEST_PHP_ARGS=-n -d -foo=1 -d zend_extension=%APPVEYOR_BUILD_FOLDER%\build\ext\php_xdebug.dll -dxdebug.remote_enable=1
	set SKIP_DBGP_TESTS=1
	set SKIP_IPV6_TESTS=1
	set REPORT_EXIT_STATUS=1
	echo !TEST_PHP_EXECUTABLE! !TEST_PHP_ARGS! -v
	echo !TEST_PHP_EXECUTABLE! -n run-tests.php -q -x --show-diff %APPVEYOR_BUILD_FOLDER%\tests
	!TEST_PHP_EXECUTABLE! !TEST_PHP_ARGS! -v
	!TEST_PHP_EXECUTABLE! -n run-tests.php -q -x --show-diff %APPVEYOR_BUILD_FOLDER%\tests

	set EXIT_CODE=%errorlevel%
	powershell -Command "$wc = New-Object 'System.Net.WebClient'; $wc.UploadFile('https://ci.appveyor.com/api/testresults/junit/%APPVEYOR_JOB_ID%', 'c:\tests_tmp\tests-junit.xml')"
	if %EXIT_CODE% neq 0 exit /b 3

	cd /d %APPVEYOR_BUILD_FOLDER%

	if not exist "%APPVEYOR_BUILD_FOLDER%\build\ext\php_xdebug.dll" exit /b 3

	xcopy %APPVEYOR_BUILD_FOLDER%\LICENSE %APPVEYOR_BUILD_FOLDER%\php_xdebug-%PHP_REL%-!ZTS_SHORT!-%PHP_BUILD_CRT%-%PHP_SDK_ARCH%\ /y /f
	echo F|xcopy %APPVEYOR_BUILD_FOLDER%\build\ext\php_xdebug.dll %APPVEYOR_BUILD_FOLDER%\php_xdebug-%PHP_REL%-!ZTS_SHORT!-%PHP_BUILD_CRT%-%PHP_SDK_ARCH%\php_xdebug-%APPVEYOR_REPO_TAG_NAME%-%PHP_REL%-%PHP_BUILD_CRT%!ZTS_IN_FILENAME!!ARCH_IN_FILENAME!.dll /y /f
	7z a php_xdebug-%APPVEYOR_REPO_TAG_NAME%-%PHP_REL%-%PHP_BUILD_CRT%!ZTS_IN_FILENAME!!ARCH_IN_FILENAME!.zip %APPVEYOR_BUILD_FOLDER%\php_xdebug-%PHP_REL%-!ZTS_SHORT!-%PHP_BUILD_CRT%-%PHP_SDK_ARCH%\*
	appveyor PushArtifact php_xdebug-%APPVEYOR_REPO_TAG_NAME%-%PHP_REL%-%PHP_BUILD_CRT%!ZTS_IN_FILENAME!!ARCH_IN_FILENAME!.zip -FileName php_xdebug-%APPVEYOR_REPO_TAG_NAME%-%PHP_REL%-%PHP_BUILD_CRT%!ZTS_IN_FILENAME!!ARCH_IN_FILENAME!.zip

	rem move build\ext\php_xdebug.dll artifacts\php_xdebug-%PHP_REL%-!ZTS_SHORT!-%PHP_BUILD_CRT%-%PHP_SDK_ARCH%.dll
endlocal
