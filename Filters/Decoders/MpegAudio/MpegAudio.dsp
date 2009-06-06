# Microsoft Developer Studio Project File - Name="MpegAudio" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=MpegAudio - Win32 Debug Fixed Point
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MpegAudio.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MpegAudio.mak" CFG="MpegAudio - Win32 Debug Fixed Point"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MpegAudio - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "MpegAudio - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "MpegAudio - Win32 Debug Fixed Point" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "MpegAudio - Win32 Release Fixed Point" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MpegAudio - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\Filter_Lib" /I "..\..\DeCSS" /I "..\..\..\GenDMOProp" /I "..\..\..\Common" /I "..\..\..\ffmpeg\libavcodec\\" /I "..\..\..\ffmpeg\\" /I "..\..\..\ffmpeg\libavutil\\" /I "..\..\..\ffmpeg\libavformat\\" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "LIBA52_DOUBLE" /D "LIBDTS_DOUBLE" /D "LIBA52_DJBFFT" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib dmoguids.lib msdmo.lib strmiids.lib avcodec-52.lib avutil-50.lib avformat-52.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"..\..\..\Debug/MpegAudio.dll" /pdbtype:sept /libpath:"..\..\..\Debug"
# Begin Custom Build - Performing registration
OutDir=.\Debug
TargetPath=\Source\deinterlace\DScaler5\Debug\MpegAudio.dll
InputPath=\Source\deinterlace\DScaler5\Debug\MpegAudio.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "MpegAudio - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "MpegAudio___Win32_Release"
# PROP BASE Intermediate_Dir "MpegAudio___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_ATL_DLL" /D "_ATL_MIN_CRT" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\Filter_Lib" /I "..\..\DeCSS" /I "..\..\..\GenDMOProp" /I "..\..\..\Common" /I "..\..\..\ffmpeg\libavcodec\\" /I "..\..\..\ffmpeg\\" /I "..\..\..\ffmpeg\libavutil\\" /I "..\..\..\ffmpeg\libavformat\\" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "LIBA52_DJBFFT" /D "LIBA52_DOUBLE" /D "NOLOGGING" /D "LIBDTS_DOUBLE" /FR /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib dmoguids.lib msdmo.lib strmiids.lib rpcrt4.lib avcodec-52.lib avutil-50.lib avformat-52.lib /nologo /subsystem:windows /dll /machine:I386 /out:"..\..\..\Release/MpegAudio.dll" /libpath:"..\..\..\Release"
# SUBTRACT LINK32 /nodefaultlib
# Begin Custom Build - Performing registration
OutDir=.\Release
TargetPath=\Source\deinterlace\DScaler5\Release\MpegAudio.dll
InputPath=\Source\deinterlace\DScaler5\Release\MpegAudio.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "MpegAudio - Win32 Debug Fixed Point"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "MpegAudio___Win32_Debug_Fixed_Point"
# PROP BASE Intermediate_Dir "MpegAudio___Win32_Debug_Fixed_Point"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Fixed_Point"
# PROP Intermediate_Dir "Debug_Fixed_Point"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\Filter_Lib" /I "..\..\..\GenDMOProp" /I "..\..\..\Common" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "LIBA52_DOUBLE" /D "LIBDTS_FIXED" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\Filter_Lib" /I "..\..\DeCSS" /I "..\..\..\GenDMOProp" /I "..\..\..\Common" /I "..\..\..\ffmpeg\libavcodec\\" /I "..\..\..\ffmpeg\\" /I "..\..\..\ffmpeg\libavutil\\" /I "..\..\..\ffmpeg\libavformat\\" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "LIBA52_FIXED" /D "LIBDTS_FIXED" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Rpcrt4.lib dmoguids.lib msdmo.lib strmiids.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"..\..\..\Debug/MpegAudio.dll" /pdbtype:sept
# ADD LINK32 ..\..\Filter_Lib\Debug\Filter_Lib.lib .\libmad\msvc++\Debug\libmad.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib dmoguids.lib msdmo.lib strmiids.lib avcodec-52.lib avutil-50.lib avformat-52.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"..\..\..\Debug/MpegAudio.dll" /pdbtype:sept /libpath:"..\..\..\Debug"
# Begin Custom Build - Performing registration
OutDir=.\Debug_Fixed_Point
TargetPath=\Source\deinterlace\DScaler5\Debug\MpegAudio.dll
InputPath=\Source\deinterlace\DScaler5\Debug\MpegAudio.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "MpegAudio - Win32 Release Fixed Point"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "MpegAudio___Win32_Release_Fixed_Point"
# PROP BASE Intermediate_Dir "MpegAudio___Win32_Release_Fixed_Point"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Fixed_Point"
# PROP Intermediate_Dir "Release_Fixed_Point"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /Od /I "..\..\Filter_Lib" /I "..\..\..\GenDMOProp" /I "..\..\..\Common" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "LIBA52_DJBFFT" /D "LIBA52_DOUBLE" /D "NOLOGGING" /D "LIBDTS_FIXED" /FR /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /Od /I "..\..\Filter_Lib" /I "..\..\DeCSS" /I "..\..\..\GenDMOProp" /I "..\..\..\Common" /I "..\..\..\ffmpeg\libavcodec\\" /I "..\..\..\ffmpeg\\" /I "..\..\..\ffmpeg\libavutil\\" /I "..\..\..\ffmpeg\libavformat\\" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "LIBA52_FIXED" /D "NOLOGGING" /D "LIBDTS_FIXED" /FR /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Rpcrt4.lib dmoguids.lib msdmo.lib strmiids.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /out:"..\..\..\Release/MpegAudio.dll"
# SUBTRACT BASE LINK32 /nodefaultlib
# ADD LINK32 ..\..\Filter_Lib\Release\Filter_Lib.lib .\libmad\msvc++\Release\libmad.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib dmoguids.lib msdmo.lib strmiids.lib avcodec-52.lib avutil-50.lib avformat-52.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /out:"..\..\..\Release/MpegAudio.dll" /libpath:"..\..\..\Release"
# SUBTRACT LINK32 /nodefaultlib
# Begin Custom Build - Performing registration
OutDir=.\Release_Fixed_Point
TargetPath=\Source\deinterlace\DScaler5\Release\MpegAudio.dll
InputPath=\Source\deinterlace\DScaler5\Release\MpegAudio.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ENDIF 

# Begin Target

# Name "MpegAudio - Win32 Debug"
# Name "MpegAudio - Win32 Release"
# Name "MpegAudio - Win32 Debug Fixed Point"
# Name "MpegAudio - Win32 Release Fixed Point"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AudioDecoder.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioDecoder_A52.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioDecoder_AAC.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioDecoder_DTS.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioDecoder_MAD.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioDecoder_PCM.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioDecoder_Rate.cpp
# End Source File
# Begin Source File

SOURCE=.\Dither.cpp
# End Source File
# Begin Source File

SOURCE=.\MpegAudio.cpp
# End Source File
# Begin Source File

SOURCE=.\MpegAudio.def
# End Source File
# Begin Source File

SOURCE=.\MpegAudio.rc
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AudioDecoder.h
# End Source File
# Begin Source File

SOURCE=.\Convert.h
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
# End Group
# Begin Group "Docs"

# PROP Default_Filter "*.txt"
# Begin Source File

SOURCE=.\Todo.txt
# End Source File
# End Group
# End Target
# End Project
