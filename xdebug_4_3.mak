# Microsoft Developer Studio Generated NMAKE File, Based on xdebug.dsp
!IF "$(CFG)" == ""
CFG=xdebug - Win32 Debug_TS
!MESSAGE No configuration specified. Defaulting to xdebug - Win32 Debug_TS.
!ENDIF 

!IF "$(CFG)" != "xdebug - Win32 Release_TS" && "$(CFG)" != "xdebug - Win32 Debug_TS"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "xdebug.mak" CFG="xdebug - Win32 Debug_TS"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "xdebug - Win32 Release_TS" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "xdebug - Win32 Debug_TS" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "xdebug - Win32 Release_TS"

OUTDIR=.\Release_TS
INTDIR=.\Release_TS
# Begin Custom Macros
OutDir=.\Release_TS
# End Custom Macros

ALL : "$(OUTDIR)\php_xdebug.dll"


CLEAN :
	-@erase "$(INTDIR)\usefulstuff.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\xdebug.obj"
	-@erase "$(INTDIR)\xdebug_code_coverage.obj"
	-@erase "$(INTDIR)\xdebug_com.obj"
	-@erase "$(INTDIR)\xdebug_handler_dbgp.obj"
	-@erase "$(INTDIR)\xdebug_handler_gdb.obj"
	-@erase "$(INTDIR)\xdebug_handler_php3.obj"
	-@erase "$(INTDIR)\xdebug_handlers.obj"
	-@erase "$(INTDIR)\xdebug_hash.obj"
	-@erase "$(INTDIR)\xdebug_llist.obj"
	-@erase "$(INTDIR)\xdebug_private.obj"
	-@erase "$(INTDIR)\xdebug_profiler.obj"
	-@erase "$(INTDIR)\xdebug_str.obj"
	-@erase "$(INTDIR)\xdebug_superglobals.obj"
	-@erase "$(INTDIR)\xdebug_var.obj"
	-@erase "$(INTDIR)\xdebug_xml.obj"
	-@erase "$(OUTDIR)\php_xdebug.dll"
	-@erase "$(OUTDIR)\php_xdebug.exp"
	-@erase "$(OUTDIR)\php_xdebug.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "..\php4" /I "..\php4\Zend" /I "..\php4\TSRM" /I "..\php4\main" /D "WIN32" /D "COMPILE_DL_XDEBUG" /D ZTS=1 /D HAVE_XDEBUG=1 /D ZEND_DEBUG=0 /D "NDEBUG" /D "_WINDOWS" /D "ZEND_WIN32" /D "PHP_WIN32" /D "HAVE_EXECUTE_DATA_PTR" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\xdebug.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=php4ts.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\php_xdebug.pdb" /machine:I386 /out:"$(OUTDIR)\php_xdebug.dll" /implib:"$(OUTDIR)\php_xdebug.lib" /libpath:"..\php4\Release_TS" /libpath:"..\php4\Release_TS_Inline" /libpath:"..\php4\Release_TS" 
LINK32_OBJS= \
	"$(INTDIR)\usefulstuff.obj" \
	"$(INTDIR)\xdebug.obj" \
	"$(INTDIR)\xdebug_code_coverage.obj" \
	"$(INTDIR)\xdebug_com.obj" \
	"$(INTDIR)\xdebug_handler_dbgp.obj" \
	"$(INTDIR)\xdebug_handler_gdb.obj" \
	"$(INTDIR)\xdebug_handler_php3.obj" \
	"$(INTDIR)\xdebug_handlers.obj" \
	"$(INTDIR)\xdebug_hash.obj" \
	"$(INTDIR)\xdebug_llist.obj" \
	"$(INTDIR)\xdebug_private.obj" \
	"$(INTDIR)\xdebug_profiler.obj" \
	"$(INTDIR)\xdebug_str.obj" \
	"$(INTDIR)\xdebug_superglobals.obj" \
	"$(INTDIR)\xdebug_var.obj" \
	"$(INTDIR)\xdebug_xml.obj"

"$(OUTDIR)\php_xdebug.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "xdebug - Win32 Debug_TS"

OUTDIR=.\Debug_TS
INTDIR=.\Debug_TS
# Begin Custom Macros
OutDir=.\Debug_TS
# End Custom Macros

ALL : "$(OUTDIR)\php_xdebug.dll"


CLEAN :
	-@erase "$(INTDIR)\usefulstuff.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\xdebug.obj"
	-@erase "$(INTDIR)\xdebug_com.obj"
	-@erase "$(INTDIR)\xdebug_code_coverage.obj"
	-@erase "$(INTDIR)\xdebug_handler_dbgp.obj"
	-@erase "$(INTDIR)\xdebug_handler_gdb.obj"
	-@erase "$(INTDIR)\xdebug_handler_php3.obj"
	-@erase "$(INTDIR)\xdebug_handlers.obj"
	-@erase "$(INTDIR)\xdebug_hash.obj"
	-@erase "$(INTDIR)\xdebug_llist.obj"
	-@erase "$(INTDIR)\xdebug_private.obj"
	-@erase "$(INTDIR)\xdebug_profiler.obj"
	-@erase "$(INTDIR)\xdebug_str.obj"
	-@erase "$(INTDIR)\xdebug_superglobals.obj"
	-@erase "$(INTDIR)\xdebug_var.obj"
	-@erase "$(INTDIR)\xdebug_xml.obj"
	-@erase "$(OUTDIR)\php_xdebug.dll"
	-@erase "$(OUTDIR)\php_xdebug.exp"
	-@erase "$(OUTDIR)\php_xdebug.ilk"
	-@erase "$(OUTDIR)\php_xdebug.lib"
	-@erase "$(OUTDIR)\php_xdebug.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\php4" /I "..\php4\Zend" /I "..\php4\TSRM" /I "..\php4\main" /D ZEND_DEBUG=1 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "COMPILE_DL_XDEBUG" /D ZTS=1 /D "ZEND_WIN32" /D "PHP_WIN32" /D HAVE_XDEBUG=1 /D "HAVE_EXECUTE_DATA_PTR" /Fp"$(INTDIR)\xdebug.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\xdebug.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=php4ts_debug.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\php_xdebug.pdb" /debug /machine:I386 /out:"$(OUTDIR)\php_xdebug.dll" /implib:"$(OUTDIR)\php_xdebug.lib" /pdbtype:sept /libpath:"..\php4\Debug_TS" 
LINK32_OBJS= \
	"$(INTDIR)\usefulstuff.obj" \
	"$(INTDIR)\xdebug.obj" \
	"$(INTDIR)\xdebug_code_coverage.obj" \
	"$(INTDIR)\xdebug_com.obj" \
	"$(INTDIR)\xdebug_handler_dbgp.obj" \
	"$(INTDIR)\xdebug_handler_gdb.obj" \
	"$(INTDIR)\xdebug_handler_php3.obj" \
	"$(INTDIR)\xdebug_handlers.obj" \
	"$(INTDIR)\xdebug_hash.obj" \
	"$(INTDIR)\xdebug_llist.obj" \
	"$(INTDIR)\xdebug_private.obj" \
	"$(INTDIR)\xdebug_profiler.obj" \
	"$(INTDIR)\xdebug_str.obj" \
	"$(INTDIR)\xdebug_superglobals.obj" \
	"$(INTDIR)\xdebug_var.obj" \
	"$(INTDIR)\xdebug_xml.obj"

"$(OUTDIR)\php_xdebug.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("xdebug.dep")
!INCLUDE "xdebug.dep"
!ELSE 
!MESSAGE Warning: cannot find "xdebug.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "xdebug - Win32 Release_TS" || "$(CFG)" == "xdebug - Win32 Debug_TS"
SOURCE=.\usefulstuff.c

"$(INTDIR)\usefulstuff.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xdebug.c

"$(INTDIR)\xdebug.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xdebug_code_coverage.c

"$(INTDIR)\xdebug_code_coverage.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xdebug_com.c

"$(INTDIR)\xdebug_com.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xdebug_handler_dbgp.c

"$(INTDIR)\xdebug_handler_dbgp.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xdebug_handler_gdb.c

"$(INTDIR)\xdebug_handler_gdb.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xdebug_handler_php3.c

"$(INTDIR)\xdebug_handler_php3.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xdebug_handlers.c

"$(INTDIR)\xdebug_handlers.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xdebug_hash.c

"$(INTDIR)\xdebug_hash.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xdebug_llist.c

"$(INTDIR)\xdebug_llist.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xdebug_private.c

"$(INTDIR)\xdebug_private.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xdebug_profiler.c

"$(INTDIR)\xdebug_profiler.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xdebug_str.c

"$(INTDIR)\xdebug_str.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xdebug_superglobals.c

"$(INTDIR)\xdebug_superglobals.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xdebug_var.c

"$(INTDIR)\xdebug_var.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xdebug_xml.c

"$(INTDIR)\xdebug_xml.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

