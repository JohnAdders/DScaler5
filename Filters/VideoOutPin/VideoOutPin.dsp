# Microsoft Developer Studio Project File - Name="VideoOutPin" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=VideoOutPin - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "VideoOutPin.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "VideoOutPin.mak" CFG="VideoOutPin - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "VideoOutPin - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "VideoOutPin - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "VideoOutPin - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\Common" /I "..\..\GenDMOProp" /I "..\Filter_Lib" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "NOLOGGING" /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "VideoOutPin - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\Common" /I "..\..\GenDMOProp" /I "..\Filter_Lib" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /Yu"stdafx.h" /FD /GZ /c
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

# Name "VideoOutPin - Win32 Release"
# Name "VideoOutPin - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\a_yuv2rgb.asm

!IF  "$(CFG)" == "VideoOutPin - Win32 Release"

# Begin Custom Build
IntDir=.\Release
InputPath=.\a_yuv2rgb.asm
InputName=a_yuv2rgb

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -o $(IntDir)\$(InputName).obj -p "..\..\Common\DScaler.mac" $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "VideoOutPin - Win32 Debug"

# Begin Custom Build
IntDir=.\Debug
InputPath=.\a_yuv2rgb.asm
InputName=a_yuv2rgb

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -o $(IntDir)\$(InputName).obj -p "..\..\Common\DScaler.mac" $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\a_yuv2yuy2.asm

!IF  "$(CFG)" == "VideoOutPin - Win32 Release"

# Begin Custom Build
IntDir=.\Release
InputPath=.\a_yuv2yuy2.asm
InputName=a_yuv2yuy2

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -o $(IntDir)\$(InputName).obj -p "..\..\Common\DScaler.mac" $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "VideoOutPin - Win32 Debug"

# Begin Custom Build
IntDir=.\Debug
InputPath=.\a_yuv2yuy2.asm
InputName=a_yuv2yuy2

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -o $(IntDir)\$(InputName).obj -p "..\..\Common\DScaler.mac" $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\a_yuvtable.asm

!IF  "$(CFG)" == "VideoOutPin - Win32 Release"

# Begin Custom Build
IntDir=.\Release
InputPath=.\a_yuvtable.asm
InputName=a_yuvtable

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -o $(IntDir)\$(InputName).obj -p "..\..\Common\DScaler.mac" $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "VideoOutPin - Win32 Debug"

# Begin Custom Build
IntDir=.\Debug
InputPath=.\a_yuvtable.asm
InputName=a_yuvtable

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -o $(IntDir)\$(InputName).obj -p "..\..\Common\DScaler.mac" $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\DSVideoOutPin.cpp
# End Source File
# Begin Source File

SOURCE=.\PlanarYUVToRGB.cpp
# End Source File
# Begin Source File

SOURCE=.\PlanarYUVToYUY2.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\DSVideoOutPin.h
# End Source File
# Begin Source File

SOURCE=.\PlanarYUVToRGB.h
# End Source File
# Begin Source File

SOURCE=.\PlanarYUVToYUY2.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Readme.txt
# End Source File
# End Target
# End Project
