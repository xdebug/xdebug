set HOMEDRIVE=C:
set HOMEPATH=\
d:
cd \php\xdebug
d:\cvs upd -dP

nmake /f xdebug_4_3.mak "CFG=xdebug - Win32 Release_TS" clean all
copy Release_TS\php_xdebug.dll c:\xdebug-4.3-2.0dev.dll

nmake /f xdebug.mak "CFG=xdebug - Win32 Release_TS" clean all
copy Release_TS\php_xdebug.dll c:\xdebug-5.0-2.0dev.dll
