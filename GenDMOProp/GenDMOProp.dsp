# Microsoft Developer Studio Project File - Name="GenDMOProp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=GenDMOProp - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GenDMOProp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GenDMOProp.mak" CFG="GenDMOProp - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GenDMOProp - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "GenDMOProp - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GenDMOProp - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\Common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  Dmoguids.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"..\Debug/GenDMOProp.dll" /pdbtype:sept
# Begin Custom Build - Performing registration
OutDir=.\Debug
TargetPath=\source\other\deinterlace\DScaler5\Debug\GenDMOProp.dll
InputPath=\source\other\deinterlace\DScaler5\Debug\GenDMOProp.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=cmd /c copy .\GenDMOProp.h ..\Common
# End Special Build Tool

!ELSEIF  "$(CFG)" == "GenDMOProp - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "GenDMOProp___Win32_Release"
# PROP BASE Intermediate_Dir "GenDMOProp___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_ATL_STATIC_REGISTRY" /D "_ATL_MIN_CRT" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O1 /I "..\Common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_ATL_STATIC_REGISTRY" /D "_ATL_MIN_CRT" /FR /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Dmoguids.lib /nologo /subsystem:windows /dll /machine:I386 /out:"..\Release/GenDMOProp.dll"
# Begin Custom Build - Performing registration
OutDir=.\Release
TargetPath=\source\other\deinterlace\DScaler5\Release\GenDMOProp.dll
InputPath=\source\other\deinterlace\DScaler5\Release\GenDMOProp.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=cmd /c copy .\GenDMOProp.h ..\Common
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "GenDMOProp - Win32 Debug"
# Name "GenDMOProp - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\GenDMOProp.cpp
# End Source File
# Begin Source File

SOURCE=.\GenDMOProp.def
# End Source File
# Begin Source File

SOURCE=.\GenDMOProp.idl
# ADD MTL /tlb ".\GenDMOProp.tlb" /h "GenDMOProp.h" /iid "GenDMOProp_i.c" /Oicf
# End Source File
# Begin Source File

SOURCE=.\GenDMOProp.rc
# End Source File
# Begin Source File

SOURCE=.\GenDMOPropPage.cpp
# End Source File
# Begin Source File

SOURCE=.\LicensePropPage.cpp
# End Source File
# Begin Source File

SOURCE=..\Common\SimplePropertyPage.cpp
# End Source File
# Begin Source File

SOURCE=.\StatisticsPropPage.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\GenDMOPropPage.h
# End Source File
# Begin Source File

SOURCE=.\LicensePropPage.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=..\Common\SimplePropertyPage.h
# End Source File
# Begin Source File

SOURCE=.\StatisticsPropPage.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
# Section GenDMOProp : {00708077-0000-0000-80B6-7826C4B70800}
# 	1:27:IDS_DOCSTRINGGenDMOPropPage:103
# 	1:18:IDD_GENDMOPROPPAGE:105
# 	1:18:IDR_GENDMOPROPPAGE:104
# 	1:23:IDS_TITLEGenDMOPropPage:101
# 	1:26:IDS_HELPFILEGenDMOPropPage:102
# End Section
# Section GenDMOProp : {00008077-0800-7778-7777-777777777777}
# 	1:19:IDD_LICENCEPROPPAGE:110
# 	1:24:IDS_TITLELicencePropPage:106
# 	1:19:IDR_LICENCEPROPPAGE:109
# 	1:27:IDS_HELPFILELicencePropPage:107
# 	1:28:IDS_DOCSTRINGLicencePropPage:108
# End Section
