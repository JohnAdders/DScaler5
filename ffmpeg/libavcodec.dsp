# Microsoft Developer Studio Project File - Name="libavcodec" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=libavcodec - Win32 Debug ICL
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libavcodec.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libavcodec.mak" CFG="libavcodec - Win32 Debug ICL"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libavcodec - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "libavcodec - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "libavcodec - Win32 Release ICL" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "libavcodec - Win32 Debug ICL" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libavcodec - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /O2 /Ob2 /I "." /I ".." /I "../codecs" /I "../imgFilters" /I "../zlib" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "HAVE_AV_CONFIG_H" /D "CONFIG_RISKY" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x41b /d "NDEBUG"
# ADD RSC /l 0x41b /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 msvcrt.lib kernel32.lib oldnames.lib /nologo /entry:"DllEntryPoint" /subsystem:windows /dll /pdb:none /machine:I386 /nodefaultlib /out:"../../bin/libavcodec.dll"

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /w /W0 /Gm /Gi /ZI /Od /I "../zlib" /I "." /I ".." /I "../codecs" /I "../imgFilters" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "HAVE_AV_CONFIG_H" /D "CONFIG_RISKY" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x41b /d "_DEBUG"
# ADD RSC /l 0x41b /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib msvcrtd.lib msvcprtd.lib oldnames.lib /nologo /entry:"DllEntryPoint" /subsystem:windows /dll /debug /machine:I386 /nodefaultlib /out:"../../bin/libavcodec.dll"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Release ICL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libavcodec___Win32_Release_ICL"
# PROP BASE Intermediate_Dir "libavcodec___Win32_Release_ICL"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_ICL"
# PROP Intermediate_Dir "Release_ICL"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MD /w /W0 /O2 /Ob2 /I "." /I ".." /I "../codecs" /I "../imgFilters" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "HAVE_AV_CONFIG_H" /D "CONFIG_RISKY" /FD /O3 /QxK /Qipo /c
# ADD CPP /nologo /G6 /MD /O2 /Ob2 /I "." /I ".." /I "../codecs" /I "../imgFilters" /I "../zlib" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "HAVE_AV_CONFIG_H" /D "CONFIG_RISKY" /FD /O3 /QxK /Qipo /Qc99 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x41b /d "NDEBUG"
# ADD RSC /l 0x41b /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 msvcrt.lib kernel32.lib libircmt.lib oldnames.lib /nologo /subsystem:windows /dll /pdb:none /machine:I386 /nodefaultlib /out:"../../bin/libavcodec.dll" /Qipo
# ADD LINK32 msvcrt.lib kernel32.lib libircmt.lib oldnames.lib libmmds.lib svml_dispmt.lib /nologo /entry:"DllEntryPoint" /subsystem:windows /dll /pdb:none /machine:I386 /nodefaultlib /out:"../../bin/libavcodec.dll" /Qipo

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug ICL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "libavcodec___Win32_Debug_ICL"
# PROP BASE Intermediate_Dir "libavcodec___Win32_Debug_ICL"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_ICL"
# PROP Intermediate_Dir "Debug_ICL"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /w /W0 /Gm /Gi /ZI /Od /I "." /I ".." /I "../codecs" /I "../imgFilters" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "HAVE_AV_CONFIG_H" /D "CONFIG_RISKY" /FR /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MDd /w /W0 /Gm /Gi /ZI /Od /I "." /I ".." /I "../codecs" /I "../imgFilters" /I "../zlib" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "HAVE_AV_CONFIG_H" /D "CONFIG_RISKY" /FR /FD /GZ /Qc99 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x41b /d "_DEBUG"
# ADD RSC /l 0x41b /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib msvcrtd.lib oldnames.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib /out:"../../bin/libavcodec.dll"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib msvcrtd.lib oldnames.lib libmmds.lib /nologo /entry:"DllEntryPoint" /subsystem:windows /dll /debug /machine:I386 /nodefaultlib /out:"../../bin/libavcodec.dll"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "libavcodec - Win32 Release"
# Name "libavcodec - Win32 Debug"
# Name "libavcodec - Win32 Release ICL"
# Name "libavcodec - Win32 Debug ICL"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\libavcodec\8bps.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\ac3enc.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\adpcm.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\allcodecs.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\amr.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\asv1.c

!IF  "$(CFG)" == "libavcodec - Win32 Release"

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug"

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Release ICL"

# ADD CPP /w /W0

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug ICL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\libavcodec\cabac.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\cinepak.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\common.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\corepng.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\cyuv.c
# End Source File
# Begin Source File

SOURCE=.\DllEntry.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\dsputil.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\dv.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\error_resilience.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\eval.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\faandct.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\fft.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\ffv1.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\flac.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\golomb.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\gsm.c

!IF  "$(CFG)" == "libavcodec - Win32 Release"

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug"

# ADD CPP /w /W0

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Release ICL"

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug ICL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\libavcodec\h261.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\h263.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\h263dec.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\h264.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\h264idct.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\huffyuv.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\imgconvert.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\indeo3.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\integer.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\jfdctfst.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\jfdctint.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\jrevdct.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\lcl.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\mdct.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\mem.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\mjpeg.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\motion_est.c

!IF  "$(CFG)" == "libavcodec - Win32 Release"

# ADD CPP /Ox /Ot /Og /Oi

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug"

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Release ICL"

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug ICL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\libavcodec\motion_est_template.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\libavcodec\mpeg12.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\mpegvideo.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\msmpeg4.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\msrle.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\msvideo1.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\my_utils.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\parser.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\pcm.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\qtrle.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\rangecoder.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\ratecontrol.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\rational.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\rv10.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\simple_idct.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\skl_dct_c.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\snow.c

!IF  "$(CFG)" == "libavcodec - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Release ICL"

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug ICL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\libavcodec\svq1.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\svq3.c

!IF  "$(CFG)" == "libavcodec - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Release ICL"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug ICL"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\libavcodec\truemotion1.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\truemotion1data.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\tscc.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\utils.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\vcr1.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\vp3.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\vp3dsp.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\w32thread.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\wmadec.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\wmv2.c

!IF  "$(CFG)" == "libavcodec - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Release ICL"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug ICL"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\libavcodec\xIdctref.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\libavcodec\liba52\a52.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\liba52\a52_internal.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\liba52\a52_util.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\ac3.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\ac3tab.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\avcodec.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\liba52\bitstream.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\bswap.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\cabac.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\common.h
# End Source File
# Begin Source File

SOURCE=.\config.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\dsputil.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\dvdata.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\faandct.h
# End Source File
# Begin Source File

SOURCE=..\codecs\ffcodecs.h
# End Source File
# Begin Source File

SOURCE=..\imgFilters\ffImgfmt.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\golomb.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\h261data.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\h263data.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\h264data.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\indeo3data.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\integer.h
# End Source File
# Begin Source File

SOURCE=..\inttypes.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\liba52\mm_accel.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\mpeg12data.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\mpeg4data.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\mpegvideo.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\msmpeg4data.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\rangecoder.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\rational.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\simple_idct.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\skl_dct.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\skl_nasm.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\sp5x.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\svq1_cb.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\svq1_vlc.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\liba52\tables.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\vp3data.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\wmadata.h
# End Source File
# End Group
# Begin Group "libavformat"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\libavformat\allformats.c
# End Source File
# Begin Source File

SOURCE=.\libavformat\avformat.h
# End Source File
# Begin Source File

SOURCE=.\libavformat\avio.c
# End Source File
# Begin Source File

SOURCE=.\libavformat\avio.h
# End Source File
# Begin Source File

SOURCE=.\libavformat\aviobuf.c
# End Source File
# Begin Source File

SOURCE=.\libavformat\cutils.c
# End Source File
# Begin Source File

SOURCE=.\libavformat\file.c
# End Source File
# Begin Source File

SOURCE=.\libavformat\mpeg.c
# End Source File
# Begin Source File

SOURCE=.\libavformat\utils_f.c
# End Source File
# End Group
# Begin Group "zlib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\zlib\adler32.c
# End Source File
# Begin Source File

SOURCE=..\zlib\crc32.c
# End Source File
# Begin Source File

SOURCE=..\zlib\crc32.h
# End Source File
# Begin Source File

SOURCE=..\zlib\infback.c
# End Source File
# Begin Source File

SOURCE=..\zlib\inffast.c
# End Source File
# Begin Source File

SOURCE=..\zlib\inffast.h
# End Source File
# Begin Source File

SOURCE=..\zlib\inffixed.h
# End Source File
# Begin Source File

SOURCE=..\zlib\inflate.c
# End Source File
# Begin Source File

SOURCE=..\zlib\inflate.h
# End Source File
# Begin Source File

SOURCE=..\zlib\inftrees.c
# End Source File
# Begin Source File

SOURCE=..\zlib\inftrees.h
# End Source File
# Begin Source File

SOURCE=..\zlib\uncompr.c
# End Source File
# Begin Source File

SOURCE=..\zlib\zconf.h
# End Source File
# Begin Source File

SOURCE=..\zlib\zlib.h
# End Source File
# Begin Source File

SOURCE=..\zlib\zutil.c
# End Source File
# Begin Source File

SOURCE=..\zlib\zutil.h
# End Source File
# End Group
# Begin Group "amr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\libavcodec\amr_float\interf_dec.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\amr_float\interf_dec.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\amr_float\interf_enc.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\amr_float\interf_rom.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\amr_float\rom_dec.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\amr_float\sp_dec.c
# End Source File
# Begin Source File

SOURCE=.\libavcodec\amr_float\sp_dec.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\amr_float\sp_enc.h
# End Source File
# Begin Source File

SOURCE=.\libavcodec\amr_float\typedef.h
# End Source File
# End Group
# Begin Group "i386"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\libavcodec\i386\cputest.c

!IF  "$(CFG)" == "libavcodec - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Release ICL"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - gcc $(InputName)
IntDir=.\Release_ICL
InputPath=.\libavcodec\i386\cputest.c
InputName=cputest

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -O3 -march=i586 -mcpu=i686 -fomit-frame-pointer -finline -finline-functions -DHAVE_AV_CONFIG_H -DCONFIG_RISKY -pipe -mno-cygwin -mdll -I. -I.. -I../codecs -I../imgFilters $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug ICL"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - gcc $(InputName)
IntDir=.\Debug_ICL
InputPath=.\libavcodec\i386\cputest.c
InputName=cputest

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -O3 -march=i586 -mcpu=i686 -fomit-frame-pointer -finline -finline-functions -DHAVE_AV_CONFIG_H -DCONFIG_RISKY -pipe -mno-cygwin -mdll -I. -I.. -I../codecs -I../imgFilters $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\libavcodec\i386\dsputil_mmx.c

!IF  "$(CFG)" == "libavcodec - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Release ICL"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - gcc $(InputName)
IntDir=.\Release_ICL
InputPath=.\libavcodec\i386\dsputil_mmx.c
InputName=dsputil_mmx

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -O3 -march=i586 -mcpu=i686 -fomit-frame-pointer -finline -finline-functions -DHAVE_AV_CONFIG_H -DCONFIG_RISKY -pipe -mno-cygwin -mdll -I. -I.. -I../codecs -I../imgFilters $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug ICL"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - gcc $(InputName)
IntDir=.\Debug_ICL
InputPath=.\libavcodec\i386\dsputil_mmx.c
InputName=dsputil_mmx

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -O3 -march=i586 -mcpu=i686 -fomit-frame-pointer -finline -finline-functions -DHAVE_AV_CONFIG_H -DCONFIG_RISKY -pipe -mno-cygwin -mdll -I. -I.. -I../codecs -I../imgFilters $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\libavcodec\i386\dsputil_mmx_avg.h

!IF  "$(CFG)" == "libavcodec - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Release ICL"

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug ICL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\libavcodec\i386\dsputil_mmx_rnd.h

!IF  "$(CFG)" == "libavcodec - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Release ICL"

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug ICL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\libavcodec\i386\fdct_mmx.c

!IF  "$(CFG)" == "libavcodec - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Release ICL"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - gcc $(InputName)
IntDir=.\Release_ICL
InputPath=.\libavcodec\i386\fdct_mmx.c
InputName=fdct_mmx

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -O3 -march=i586 -mcpu=i686 -fomit-frame-pointer -finline -finline-functions -DHAVE_AV_CONFIG_H -DCONFIG_RISKY -pipe -mno-cygwin -mdll -I. -I.. -I../codecs -I../imgFilters $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug ICL"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - gcc $(InputName)
IntDir=.\Debug_ICL
InputPath=.\libavcodec\i386\fdct_mmx.c
InputName=fdct_mmx

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -O3 -march=i586 -mcpu=i686 -fomit-frame-pointer -finline -finline-functions -DHAVE_AV_CONFIG_H -DCONFIG_RISKY -pipe -mno-cygwin -mdll -I. -I.. -I../codecs -I../imgFilters $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\libavcodec\i386\fft_sse.c

!IF  "$(CFG)" == "libavcodec - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Release ICL"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - gcc $(InputName)
IntDir=.\Release_ICL
InputPath=.\libavcodec\i386\fft_sse.c
InputName=fft_sse

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -O3 -msse -march=i586 -mcpu=i686 -fomit-frame-pointer -finline -finline-functions -DHAVE_AV_CONFIG_H -DCONFIG_RISKY -pipe -mno-cygwin -mdll -I. -I.. -I../codecs -I../imgFilters $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug ICL"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - gcc $(InputName)
IntDir=.\Debug_ICL
InputPath=.\libavcodec\i386\fft_sse.c
InputName=fft_sse

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -O3 -msse -march=i586 -mcpu=i686 -fomit-frame-pointer -finline -finline-functions -DHAVE_AV_CONFIG_H -DCONFIG_RISKY -pipe -mno-cygwin -mdll -I. -I.. -I../codecs -I../imgFilters $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\libavcodec\i386\idct_mmx.c

!IF  "$(CFG)" == "libavcodec - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Release ICL"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - gcc $(InputName)
IntDir=.\Release_ICL
InputPath=.\libavcodec\i386\idct_mmx.c
InputName=idct_mmx

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -O3 -march=i586 -mcpu=i686 -fomit-frame-pointer -finline -finline-functions -DHAVE_AV_CONFIG_H -DCONFIG_RISKY -pipe -mno-cygwin -mdll -I. -I.. -I../codecs -I../imgFilters $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug ICL"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - gcc $(InputName)
IntDir=.\Debug_ICL
InputPath=.\libavcodec\i386\idct_mmx.c
InputName=idct_mmx

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -O3 -march=i586 -mcpu=i686 -fomit-frame-pointer -finline -finline-functions -DHAVE_AV_CONFIG_H -DCONFIG_RISKY -pipe -mno-cygwin -mdll -I. -I.. -I../codecs -I../imgFilters $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\libavcodec\i386\mmx.h

!IF  "$(CFG)" == "libavcodec - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Release ICL"

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug ICL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\libavcodec\i386\motion_est_mmx.c

!IF  "$(CFG)" == "libavcodec - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Release ICL"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - gcc $(InputName)
IntDir=.\Release_ICL
InputPath=.\libavcodec\i386\motion_est_mmx.c
InputName=motion_est_mmx

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -O3 -march=i586 -mcpu=i686 -fomit-frame-pointer -finline -finline-functions -DHAVE_AV_CONFIG_H -DCONFIG_RISKY -pipe -mno-cygwin -mdll -I. -I.. -I../codecs -I../imgFilters $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug ICL"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - gcc $(InputName)
IntDir=.\Debug_ICL
InputPath=.\libavcodec\i386\motion_est_mmx.c
InputName=motion_est_mmx

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -O3 -march=i586 -mcpu=i686 -fomit-frame-pointer -finline -finline-functions -DHAVE_AV_CONFIG_H -DCONFIG_RISKY -pipe -mno-cygwin -mdll -I. -I.. -I../codecs -I../imgFilters $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\libavcodec\i386\mpegvideo_mmx.c

!IF  "$(CFG)" == "libavcodec - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Release ICL"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - gcc $(InputName)
IntDir=.\Release_ICL
InputPath=.\libavcodec\i386\mpegvideo_mmx.c
InputName=mpegvideo_mmx

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -O3 -march=i586 -mcpu=i686 -fomit-frame-pointer -finline -finline-functions -DHAVE_AV_CONFIG_H -DCONFIG_RISKY -pipe -mno-cygwin -mdll -I. -I.. -I../codecs -I../imgFilters $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug ICL"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - gcc $(InputName)
IntDir=.\Debug_ICL
InputPath=.\libavcodec\i386\mpegvideo_mmx.c
InputName=mpegvideo_mmx

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -O3 -march=i586 -mcpu=i686 -fomit-frame-pointer -finline -finline-functions -DHAVE_AV_CONFIG_H -DCONFIG_RISKY -pipe -mno-cygwin -mdll -I. -I.. -I../codecs -I../imgFilters $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\libavcodec\i386\simple_idct_mmx.c

!IF  "$(CFG)" == "libavcodec - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Release ICL"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - gcc $(InputName)
IntDir=.\Release_ICL
InputPath=.\libavcodec\i386\simple_idct_mmx.c
InputName=simple_idct_mmx

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -O3 -march=i586 -mcpu=i686 -fomit-frame-pointer -finline -finline-functions -DHAVE_AV_CONFIG_H -DCONFIG_RISKY -pipe -mno-cygwin -mdll -I. -I.. -I../codecs -I../imgFilters $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug ICL"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - gcc $(InputName)
IntDir=.\Debug_ICL
InputPath=.\libavcodec\i386\simple_idct_mmx.c
InputName=simple_idct_mmx

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -O3 -march=i586 -mcpu=i686 -fomit-frame-pointer -finline -finline-functions -DHAVE_AV_CONFIG_H -DCONFIG_RISKY -pipe -mno-cygwin -mdll -I. -I.. -I../codecs -I../imgFilters $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\libavcodec\i386\skl_dct_sse.asm

!IF  "$(CFG)" == "libavcodec - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Release ICL"

# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release_ICL
InputPath=.\libavcodec\i386\skl_dct_sse.asm
InputName=skl_dct_sse

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm -f win32 -DPREFIX -DWIN32 -Ilibavcodec/i386/ -o $(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug ICL"

# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug_ICL
InputPath=.\libavcodec\i386\skl_dct_sse.asm
InputName=skl_dct_sse

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm -f win32 -DPREFIX -DWIN32 -Ilibavcodec/i386/ -o $(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\libavcodec\i386\skl_dct_sse2.asm

!IF  "$(CFG)" == "libavcodec - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Release ICL"

# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release_ICL
InputPath=.\libavcodec\i386\skl_dct_sse2.asm
InputName=skl_dct_sse2

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm -f win32 -DPREFIX -DWIN32 -Ilibavcodec/i386/ -o $(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug ICL"

# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug_ICL
InputPath=.\libavcodec\i386\skl_dct_sse2.asm
InputName=skl_dct_sse2

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm -f win32 -DPREFIX -DWIN32 -Ilibavcodec/i386/ -o $(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\libavcodec\i386\skl_nasm.h

!IF  "$(CFG)" == "libavcodec - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Release ICL"

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug ICL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\libavcodec\i386\vp3dsp_mmx.c

!IF  "$(CFG)" == "libavcodec - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Release ICL"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - gcc $(InputName)
IntDir=.\Release_ICL
InputPath=.\libavcodec\i386\vp3dsp_mmx.c
InputName=vp3dsp_mmx

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -O3 -march=i586 -mcpu=i686 -fomit-frame-pointer -finline -finline-functions -DHAVE_AV_CONFIG_H -DCONFIG_RISKY -pipe -mno-cygwin -mdll -I. -I.. -I../codecs -I../imgFilters $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug ICL"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - gcc $(InputName)
IntDir=.\Debug_ICL
InputPath=.\libavcodec\i386\vp3dsp_mmx.c
InputName=vp3dsp_mmx

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -O3 -march=i586 -mcpu=i686 -fomit-frame-pointer -finline -finline-functions -DHAVE_AV_CONFIG_H -DCONFIG_RISKY -pipe -mno-cygwin -mdll -I. -I.. -I../codecs -I../imgFilters $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\libavcodec\i386\vp3dsp_sse2.c

!IF  "$(CFG)" == "libavcodec - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Release ICL"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - gcc $(InputName)
IntDir=.\Release_ICL
InputPath=.\libavcodec\i386\vp3dsp_sse2.c
InputName=vp3dsp_sse2

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -O3 -march=i586 -mcpu=i686 -fomit-frame-pointer -finline -finline-functions -DHAVE_AV_CONFIG_H -DCONFIG_RISKY -pipe -mno-cygwin -mdll -I. -I.. -I../codecs -I../imgFilters $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "libavcodec - Win32 Debug ICL"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - gcc $(InputName)
IntDir=.\Debug_ICL
InputPath=.\libavcodec\i386\vp3dsp_sse2.c
InputName=vp3dsp_sse2

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	gcc -c -O3 -march=i586 -mcpu=i686 -fomit-frame-pointer -finline -finline-functions -DHAVE_AV_CONFIG_H -DCONFIG_RISKY -pipe -mno-cygwin -mdll -I. -I.. -I../codecs -I../imgFilters $(InputPath) -o $(IntDir)\$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "png"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\png\png.c
# End Source File
# Begin Source File

SOURCE=..\png\png.h
# End Source File
# Begin Source File

SOURCE=..\png\pngconf.h
# End Source File
# Begin Source File

SOURCE=..\png\pngread.c
# End Source File
# Begin Source File

SOURCE=..\png\pngrutil.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\libavcodec.def
# End Source File
# End Target
# End Project
