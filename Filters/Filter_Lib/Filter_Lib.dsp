# Microsoft Developer Studio Project File - Name="Filter_Lib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Filter_Lib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Filter_Lib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Filter_Lib.mak" CFG="Filter_Lib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Filter_Lib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Filter_Lib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Filter_Lib - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\Common" /I "..\..\GenDMOProp" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "NOLOGGING" /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Filter_Lib - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\Common" /I "..\..\GenDMOProp" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /Yu"stdafx.h" /FD /GZ /c
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

# Name "Filter_Lib - Win32 Release"
# Name "Filter_Lib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\Common\CPUID.asm

!IF  "$(CFG)" == "Filter_Lib - Win32 Release"

# Begin Custom Build
IntDir=.\Release
InputPath=..\..\Common\CPUID.asm
InputName=CPUID

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm -f win32 -o $(IntDir)\$(InputName).obj -p "..\..\Common\DScaler.mac" $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "Filter_Lib - Win32 Debug"

# Begin Custom Build
IntDir=.\Debug
InputPath=..\..\Common\CPUID.asm
InputName=CPUID

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm -f win32 -o $(IntDir)\$(InputName).obj -p "..\..\Common\DScaler.mac" $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\DSBaseFilter.cpp
# End Source File
# Begin Source File

SOURCE=.\DSBasePin.cpp
# End Source File
# Begin Source File

SOURCE=.\DSBufferedInputPin.cpp
# End Source File
# Begin Source File

SOURCE=.\DSInputPin.cpp
# End Source File
# Begin Source File

SOURCE=.\DSOutputPin.cpp
# End Source File
# Begin Source File

SOURCE=.\DSUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\EnumMediaTypes.cpp
# End Source File
# Begin Source File

SOURCE=.\EnumPins.cpp
# End Source File
# Begin Source File

SOURCE=.\InputMemAlloc.cpp
# End Source File
# Begin Source File

SOURCE=.\MediaBufferWrapper.cpp
# End Source File
# Begin Source File

SOURCE=.\MediaTypes.cpp
# End Source File
# Begin Source File

SOURCE=.\MoreUuids.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Utils.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\DSBaseFilter.h
# End Source File
# Begin Source File

SOURCE=.\DSBasePin.h
# End Source File
# Begin Source File

SOURCE=.\DSBufferedInputPin.h
# End Source File
# Begin Source File

SOURCE=.\DSInputPin.h
# End Source File
# Begin Source File

SOURCE=.\DSOutputPin.h
# End Source File
# Begin Source File

SOURCE=.\DSUtil.h
# End Source File
# Begin Source File

SOURCE=.\EnumMediaTypes.h
# End Source File
# Begin Source File

SOURCE=.\EnumPins.h
# End Source File
# Begin Source File

SOURCE=.\InputMemAlloc.h
# End Source File
# Begin Source File

SOURCE=.\MediaBufferWrapper.h
# End Source File
# Begin Source File

SOURCE=.\MediaTypes.h
# End Source File
# Begin Source File

SOURCE=.\MoreUuids.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\Utils.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Readme.txt
# End Source File
# End Target
# End Project
