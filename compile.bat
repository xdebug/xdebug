set HOMEDRIVE=C:
set HOMEPATH=\
d:
cd \php\xdebug
d:\cvs upd -dP

nmake /f xdebug_4_3.mak "CFG=xdebug - Win32 Release_TS" clean all
copy Release_TS\php_xdebug.dll h:\php_xdebug_4_3.dll

nmake /f xdebug.mak "CFG=xdebug - Win32 Release_TS" clean all
copy Release_TS\php_xdebug.dll h:\

