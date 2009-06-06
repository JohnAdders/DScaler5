# Microsoft Developer Studio Project File - Name="liba52" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=liba52 - Win32 Debug Fixed Point
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "liba52.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "liba52.mak" CFG="liba52 - Win32 Debug Fixed Point"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "liba52 - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "liba52 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "liba52 - Win32 Debug Fixed Point" (based on "Win32 (x86) Static Library")
!MESSAGE "liba52 - Win32 Release Fixed Point" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "liba52 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "." /I "../include" /I "../../djbfft" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "LIBA52_DOUBLE" /D "LIBA52_DJBFFT" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "liba52 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "." /I "../include" /I "../../djbfft" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "LIBA52_DOUBLE" /D "LIBA52_DJBFFT" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "liba52 - Win32 Debug Fixed Point"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "liba52___Win32_Debug_Fixed_Point"
# PROP BASE Intermediate_Dir "liba52___Win32_Debug_Fixed_Point"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Fixed_Point"
# PROP Intermediate_Dir "Debug_Fixed_Point"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "." /I "../include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "LIBA52_DOUBLE" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "." /I "../include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "LIBA52_FIXED" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "liba52 - Win32 Release Fixed Point"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "liba52___Win32_Release_Fixed_Point"
# PROP BASE Intermediate_Dir "liba52___Win32_Release_Fixed_Point"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Fixed_Point"
# PROP Intermediate_Dir "Release_Fixed_Point"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /I "." /I "../include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "." /I "../include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "LIBA52_FIXED" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "liba52 - Win32 Release"
# Name "liba52 - Win32 Debug"
# Name "liba52 - Win32 Debug Fixed Point"
# Name "liba52 - Win32 Release Fixed Point"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\liba52\bit_allocate.c
# End Source File
# Begin Source File

SOURCE=..\liba52\bitstream.c
# End Source File
# Begin Source File

SOURCE=..\liba52\downmix.c
# End Source File
# Begin Source File

SOURCE=..\liba52\imdct.c
# End Source File
# Begin Source File

SOURCE=..\liba52\parse.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\include\a52.h
# End Source File
# Begin Source File

SOURCE=..\liba52\a52_internal.h
# End Source File
# Begin Source File

SOURCE=..\liba52\bitstream.h
# End Source File
# Begin Source File

SOURCE=..\liba52\tables.h
# End Source File
# End Group
# End Target
# End Project
