# Microsoft Developer Studio Project File - Name="djbfft" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=djbfft - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "djbfft.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "djbfft.mak" CFG="djbfft - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "djbfft - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "djbfft - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "djbfft - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "djbfft - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
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

# Name "djbfft - Win32 Release"
# Name "djbfft - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\4c0.c
# End Source File
# Begin Source File

SOURCE=.\4c1.c
# End Source File
# Begin Source File

SOURCE=.\4c2.c
# End Source File
# Begin Source File

SOURCE=.\4c3.c
# End Source File
# Begin Source File

SOURCE=.\4c4.c
# End Source File
# Begin Source File

SOURCE=.\4c5.c
# End Source File
# Begin Source File

SOURCE=.\4d0.c
# End Source File
# Begin Source File

SOURCE=.\4d1.c
# End Source File
# Begin Source File

SOURCE=.\4d2.c
# End Source File
# Begin Source File

SOURCE=.\4d3.c
# End Source File
# Begin Source File

SOURCE=.\4d4.c
# End Source File
# Begin Source File

SOURCE=.\4d5.c
# End Source File
# Begin Source File

SOURCE=.\4mc.c
# End Source File
# Begin Source File

SOURCE=.\4mr.c
# End Source File
# Begin Source File

SOURCE=.\4r0.c
# End Source File
# Begin Source File

SOURCE=.\4r1.c
# End Source File
# Begin Source File

SOURCE=.\4r2.c
# End Source File
# Begin Source File

SOURCE=.\4r3.c
# End Source File
# Begin Source File

SOURCE=.\4r4.c
# End Source File
# Begin Source File

SOURCE=.\4r5.c
# End Source File
# Begin Source File

SOURCE=.\4sc.c
# End Source File
# Begin Source File

SOURCE=.\4sr.c
# End Source File
# Begin Source File

SOURCE=.\4u0.c
# End Source File
# Begin Source File

SOURCE=.\4u1.c
# End Source File
# Begin Source File

SOURCE=.\4u2.c
# End Source File
# Begin Source File

SOURCE=.\4u3.c
# End Source File
# Begin Source File

SOURCE=.\4u4.c
# End Source File
# Begin Source File

SOURCE=.\4u5.c
# End Source File
# Begin Source File

SOURCE=.\4v0.c
# End Source File
# Begin Source File

SOURCE=.\4v1.c
# End Source File
# Begin Source File

SOURCE=.\4v2.c
# End Source File
# Begin Source File

SOURCE=.\4v3.c
# End Source File
# Begin Source File

SOURCE=.\4v4.c
# End Source File
# Begin Source File

SOURCE=.\4v5.c
# End Source File
# Begin Source File

SOURCE=.\8c0.c
# End Source File
# Begin Source File

SOURCE=.\8c1.c
# End Source File
# Begin Source File

SOURCE=.\8c2.c
# End Source File
# Begin Source File

SOURCE=.\8c3.c
# End Source File
# Begin Source File

SOURCE=.\8c4.c
# End Source File
# Begin Source File

SOURCE=.\8c5.c
# End Source File
# Begin Source File

SOURCE=.\8d0.c
# End Source File
# Begin Source File

SOURCE=.\8d1.c
# End Source File
# Begin Source File

SOURCE=.\8d2.c
# End Source File
# Begin Source File

SOURCE=.\8d3.c
# End Source File
# Begin Source File

SOURCE=.\8d4.c
# End Source File
# Begin Source File

SOURCE=.\8d5.c
# End Source File
# Begin Source File

SOURCE=.\8mc.c
# End Source File
# Begin Source File

SOURCE=.\8mr.c
# End Source File
# Begin Source File

SOURCE=.\8r0.c
# End Source File
# Begin Source File

SOURCE=.\8r1.c
# End Source File
# Begin Source File

SOURCE=.\8r2.c
# End Source File
# Begin Source File

SOURCE=.\8r3.c
# End Source File
# Begin Source File

SOURCE=.\8r4.c
# End Source File
# Begin Source File

SOURCE=.\8r5.c
# End Source File
# Begin Source File

SOURCE=.\8sc.c
# End Source File
# Begin Source File

SOURCE=.\8sr.c
# End Source File
# Begin Source File

SOURCE=.\8u0.c
# End Source File
# Begin Source File

SOURCE=.\8u1.c
# End Source File
# Begin Source File

SOURCE=.\8u2.c
# End Source File
# Begin Source File

SOURCE=.\8u3.c
# End Source File
# Begin Source File

SOURCE=.\8u4.c
# End Source File
# Begin Source File

SOURCE=.\8u5.c
# End Source File
# Begin Source File

SOURCE=.\8v0.c
# End Source File
# Begin Source File

SOURCE=.\8v1.c
# End Source File
# Begin Source File

SOURCE=.\8v2.c
# End Source File
# Begin Source File

SOURCE=.\8v3.c
# End Source File
# Begin Source File

SOURCE=.\8v4.c
# End Source File
# Begin Source File

SOURCE=.\8v5.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\complex4.h
# End Source File
# Begin Source File

SOURCE=.\complex8.h
# End Source File
# Begin Source File

SOURCE=.\fftc4.h
# End Source File
# Begin Source File

SOURCE=.\fftc8.h
# End Source File
# Begin Source File

SOURCE=.\fftr4.h
# End Source File
# Begin Source File

SOURCE=.\fftr8.h
# End Source File
# Begin Source File

SOURCE=.\real4.h
# End Source File
# Begin Source File

SOURCE=.\real8.h
# End Source File
# End Group
# End Target
# End Project
