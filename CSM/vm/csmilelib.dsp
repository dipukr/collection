# Microsoft Developer Studio Project File - Name="csmilelib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=csmilelib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "csmilelib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "csmilelib.mak" CFG="csmilelib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "csmilelib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "csmilelib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "csmilelib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\build\Release"
# PROP Intermediate_Dir "..\build\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O1 /I "..\sal" /I "..\tool" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "COMPILER" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "csmilelib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\build\Debug"
# PROP Intermediate_Dir "..\build\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\sal" /I "..\tool" /I ".." /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "COMPILER" /D "SHOW_GC" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "csmilelib - Win32 Release"
# Name "csmilelib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\vm\archive.cpp
# End Source File
# Begin Source File

SOURCE=".\c-smile-ni.cpp"
# End Source File
# Begin Source File

SOURCE=..\vm\compiler.cpp
# End Source File
# Begin Source File

SOURCE=..\vm\cs_main.cpp
# End Source File
# Begin Source File

SOURCE=..\vm\debug.cpp
# End Source File
# Begin Source File

SOURCE=..\vm\mm.cpp
# End Source File
# Begin Source File

SOURCE=..\vm\rtl.cpp
# End Source File
# Begin Source File

SOURCE=.\rtl_array.cpp
# End Source File
# Begin Source File

SOURCE=.\rtl_blob.cpp
# End Source File
# Begin Source File

SOURCE=.\rtl_date.cpp
# End Source File
# Begin Source File

SOURCE=.\rtl_file.cpp
# End Source File
# Begin Source File

SOURCE=.\rtl_map.cpp
# End Source File
# Begin Source File

SOURCE=.\rtl_regexp.cpp
# End Source File
# Begin Source File

SOURCE=.\rtl_socket.cpp
# End Source File
# Begin Source File

SOURCE=.\rtl_stream.cpp
# End Source File
# Begin Source File

SOURCE=.\rtl_string.cpp
# End Source File
# Begin Source File

SOURCE=..\vm\scanner.cpp
# End Source File
# Begin Source File

SOURCE=..\vm\streams.cpp
# End Source File
# Begin Source File

SOURCE=..\vm\sym_table.cpp
# End Source File
# Begin Source File

SOURCE=..\vm\things.cpp
# End Source File
# Begin Source File

SOURCE=..\vm\threads.cpp
# End Source File
# Begin Source File

SOURCE=..\vm\vm.cpp
# End Source File
# Begin Source File

SOURCE=..\vm\vm_interpret.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\vm\arithmetic.h
# End Source File
# Begin Source File

SOURCE=".\c-smile.h"
# End Source File
# Begin Source File

SOURCE=..\vm\compiler.h
# End Source File
# Begin Source File

SOURCE=..\vm\mm.h
# End Source File
# Begin Source File

SOURCE=..\vm\opcodes.h
# End Source File
# Begin Source File

SOURCE=..\vm\rtl.h
# End Source File
# Begin Source File

SOURCE=..\vm\scanner.h
# End Source File
# Begin Source File

SOURCE=..\vm\streams.h
# End Source File
# Begin Source File

SOURCE=..\vm\sym_table.h
# End Source File
# Begin Source File

SOURCE=..\vm\vm.h
# End Source File
# End Group
# End Target
# End Project
