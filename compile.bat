set HOMEDRIVE=C:
set HOMEPATH=\
d:
cd \php\xdebug
d:\cvs upd
nmake /f xdebug.mak "CFG=xdebug - Win32 Release_TS" clean all
copy Release_TS\php_xdebug.dll h:\
