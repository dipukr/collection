# Microsoft Developer Studio Project File - Name="tool" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=TOOL - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TOOL.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TOOL.mak" CFG="TOOL - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "tool - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "tool - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "tool - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "tool - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\tool" /I ".." /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
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

# Name "tool - Win32 Release"
# Name "tool - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\cs_base64.cpp
# End Source File
# Begin Source File

SOURCE=.\cs_datetime.cpp
# End Source File
# Begin Source File

SOURCE=.\cs_lzf.cpp
# End Source File
# Begin Source File

SOURCE=.\cs_parser.cpp
# End Source File
# Begin Source File

SOURCE=.\cs_regexp.cpp
# End Source File
# Begin Source File

SOURCE=.\cs_STRING.CPP
# End Source File
# Begin Source File

SOURCE=.\cs_url.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\cs_array.h
# End Source File
# Begin Source File

SOURCE=.\cs_base64.h
# End Source File
# Begin Source File

SOURCE=.\cs_BASIC.H
# End Source File
# Begin Source File

SOURCE=.\cs_datetime.h
# End Source File
# Begin Source File

SOURCE=.\cs_dictionary.h
# End Source File
# Begin Source File

SOURCE=.\cs_hash.h
# End Source File
# Begin Source File

SOURCE=.\cs_HASH_TABLE.H
# End Source File
# Begin Source File

SOURCE=.\cs_html.h
# End Source File
# Begin Source File

SOURCE=.\cs_LIST.H
# End Source File
# Begin Source File

SOURCE=.\cs_lzf.h
# End Source File
# Begin Source File

SOURCE=.\cs_ml_scanner.h
# End Source File
# Begin Source File

SOURCE=.\cs_parser.h
# End Source File
# Begin Source File

SOURCE=.\cs_regexp.h
# End Source File
# Begin Source File

SOURCE=.\cs_STRING.H
# End Source File
# Begin Source File

SOURCE=.\cs_url.h
# End Source File
# Begin Source File

SOURCE=.\cs_ustring.h
# End Source File
# Begin Source File

SOURCE=.\tool
# End Source File
# Begin Source File

SOURCE=.\xml_in_stream.h
# End Source File
# Begin Source File

SOURCE=.\xml_out_stream.h
# End Source File
# End Group
# End Target
# End Project
