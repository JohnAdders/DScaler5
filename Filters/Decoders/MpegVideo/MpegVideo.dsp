# Microsoft Developer Studio Project File - Name="MpegVideo" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=MpegVideo - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MpegVideo.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MpegVideo.mak" CFG="MpegVideo - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MpegVideo - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "MpegVideo - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MpegVideo - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\Filter_Lib" /I "..\..\..\GenDMOProp" /I "..\..\..\Common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Rpcrt4.lib dxerr9.lib dmoguids.lib msdmo.lib strmiids.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"..\..\..\Debug/MpegVideo.dll" /pdbtype:sept
# Begin Custom Build - Performing registration
OutDir=.\Debug
TargetPath=\Source\deinterlace\DScaler5\Debug\MpegVideo.dll
InputPath=\Source\deinterlace\DScaler5\Debug\MpegVideo.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "MpegVideo - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "MpegVideo___Win32_Release"
# PROP BASE Intermediate_Dir "MpegVideo___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_ATL_DLL" /D "_ATL_MIN_CRT" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\Filter_Lib" /I "..\..\..\GenDMOProp" /I "..\..\..\Common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NOLOGGING" /FR /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Rpcrt4.lib dxerr9.lib dmoguids.lib msdmo.lib strmiids.lib /nologo /subsystem:windows /dll /machine:I386 /out:"..\..\..\Release/MpegVideo.dll"
# Begin Custom Build - Performing registration
OutDir=.\Release
TargetPath=\Source\deinterlace\DScaler5\Release\MpegVideo.dll
InputPath=\Source\deinterlace\DScaler5\Release\MpegVideo.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ENDIF 

# Begin Target

# Name "MpegVideo - Win32 Debug"
# Name "MpegVideo - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\a_yuv2rgb.asm

!IF  "$(CFG)" == "MpegVideo - Win32 Debug"

# Begin Custom Build
IntDir=.\Debug
InputPath=.\a_yuv2rgb.asm
InputName=a_yuv2rgb

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -o $(IntDir)\$(InputName).obj -p "..\..\..\Common\DScaler.mac" $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "MpegVideo - Win32 Release"

# Begin Custom Build
IntDir=.\Release
InputPath=.\a_yuv2rgb.asm
InputName=a_yuv2rgb

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -o $(IntDir)\$(InputName).obj -p "..\..\..\Common\DScaler.mac" $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\a_yuv2yuy2.asm

!IF  "$(CFG)" == "MpegVideo - Win32 Debug"

# Begin Custom Build
IntDir=.\Debug
InputPath=.\a_yuv2yuy2.asm
InputName=a_yuv2yuy2

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -o $(IntDir)\$(InputName).obj -p "..\..\..\Common\DScaler.mac" $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "MpegVideo - Win32 Release"

# Begin Custom Build
IntDir=.\Release
InputPath=.\a_yuv2yuy2.asm
InputName=a_yuv2yuy2

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -o $(IntDir)\$(InputName).obj -p "..\..\..\Common\DScaler.mac" $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\a_yuvtable.asm

!IF  "$(CFG)" == "MpegVideo - Win32 Debug"

# Begin Custom Build
IntDir=.\Debug
InputPath=.\a_yuvtable.asm
InputName=a_yuvtable

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -o $(IntDir)\$(InputName).obj -p "..\..\..\Common\DScaler.mac" $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "MpegVideo - Win32 Release"

# Begin Custom Build
IntDir=.\Release
InputPath=.\a_yuvtable.asm
InputName=a_yuvtable

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -o $(IntDir)\$(InputName).obj -p "..\..\..\Common\DScaler.mac" $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\DSUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\FrameBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\MediaTypes.cpp
# End Source File
# Begin Source File

SOURCE=.\MpegDecoder.cpp
# End Source File
# Begin Source File

SOURCE=.\MpegDecoder_Rate.cpp
# End Source File
# Begin Source File

SOURCE=.\MpegDecoder_SubPic.cpp
# End Source File
# Begin Source File

SOURCE=.\MpegDecoder_UserData.cpp
# End Source File
# Begin Source File

SOURCE=.\MpegVideo.cpp
# End Source File
# Begin Source File

SOURCE=.\MpegVideo.def
# End Source File
# Begin Source File

SOURCE=.\MpegVideo.rc
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

SOURCE=.\DScaler.h
# End Source File
# Begin Source File

SOURCE=.\DSUtil.h
# End Source File
# Begin Source File

SOURCE=.\MediaTypes.h
# End Source File
# Begin Source File

SOURCE=.\moreuuids.h
# End Source File
# Begin Source File

SOURCE=.\MpegDecoder.h
# End Source File
# Begin Source File

SOURCE=.\PlanarYUVToRGB.h
# End Source File
# Begin Source File

SOURCE=.\PlanarYUVToYUY2.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\bitmap1.bmp
# End Source File
# End Group
# Begin Group "Docs"

# PROP Default_Filter "*.txt"
# Begin Source File

SOURCE=.\Todo.txt
# End Source File
# End Group
# End Target
# End Project
