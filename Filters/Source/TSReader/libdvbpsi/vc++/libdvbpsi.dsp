# Microsoft Developer Studio Project File - Name="libdvbpsi" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libdvbpsi - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libdvbpsi.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libdvbpsi.mak" CFG="libdvbpsi - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libdvbpsi - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libdvbpsi - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libdvbpsi - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "." /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libdvbpsi - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "." /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "libdvbpsi - Win32 Release"
# Name "libdvbpsi - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\demux.c
# End Source File
# Begin Source File

SOURCE=..\src\descriptor.c
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_02.c
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_03.c
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_04.c
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_05.c
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_06.c
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_07.c
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_08.c
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_09.c
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_0a.c
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_0b.c
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_0c.c
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_0d.c
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_0e.c
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_0f.c
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_42.c
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_47.c
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_48.c
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_59.c
# End Source File
# Begin Source File

SOURCE=..\src\dvbpsi.c
# End Source File
# Begin Source File

SOURCE=.\Logging.c
# End Source File
# Begin Source File

SOURCE=..\src\tables\pat.c
# End Source File
# Begin Source File

SOURCE=..\src\tables\pmt.c
# End Source File
# Begin Source File

SOURCE=..\src\psi.c
# End Source File
# Begin Source File

SOURCE=..\src\tables\sdt.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\config.h
# End Source File
# Begin Source File

SOURCE=..\src\demux.h
# End Source File
# Begin Source File

SOURCE=..\src\descriptor.h
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr.h
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_02.h
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_03.h
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_04.h
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_05.h
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_06.h
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_07.h
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_08.h
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_09.h
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_0a.h
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_0b.h
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_0c.h
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_0d.h
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_0e.h
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_0f.h
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_42.h
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_47.h
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_48.h
# End Source File
# Begin Source File

SOURCE=..\src\descriptors\dr_59.h
# End Source File
# Begin Source File

SOURCE=..\src\dvbpsi.h
# End Source File
# Begin Source File

SOURCE=..\src\dvbpsi_private.h
# End Source File
# Begin Source File

SOURCE=.\inttypes.h
# End Source File
# Begin Source File

SOURCE=..\src\tables\pat.h
# End Source File
# Begin Source File

SOURCE=..\src\tables\pat_private.h
# End Source File
# Begin Source File

SOURCE=..\src\tables\pmt.h
# End Source File
# Begin Source File

SOURCE=..\src\tables\pmt_private.h
# End Source File
# Begin Source File

SOURCE=..\src\psi.h
# End Source File
# Begin Source File

SOURCE=..\src\tables\sdt.h
# End Source File
# Begin Source File

SOURCE=..\src\tables\sdt_private.h
# End Source File
# End Group
# End Target
# End Project
