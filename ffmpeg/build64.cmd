@echo off
rem Hopefully a temporary workaround for getting MSYS shell to run on x64
rem (WoW64 cmd prompt sets PROCESSOR_ARCHITECTURE to x86)
if not "x%PROCESSOR_ARCHITECTURE%" == "xAMD64" goto _NotX64
set COMSPEC=%WINDIR%\SysWOW64\cmd.exe
%COMSPEC% /c %0 %1 %2 %3 %4 %5 %6 %7 %8 %9
goto EOF
:_NotX64
rem needs to be set to your location 
if "x%MSYSROOT%" == "x" set MSYSROOT=c:\msys64\
rem VS6 sets up MSDevDir
if not "x%MSDevDir%" == "x" set call "%MSDevDir%\..\..\VC98\bin\vcvars32.bat"
rem VS2005 sets up VS80COMNTOOLS
if not "x%VS80COMNTOOLS%" == "x" call "%VS80COMNTOOLS%vsvars32.bat"
rem VS2008 sets up VS90COMNTOOLS
if not "x%VS90COMNTOOLS%" == "x" call "%VS90COMNTOOLS%vsvars32.bat"
set MSYSTEM=MINGW32
set DISPLAY=
set PATH=%MSYSROOT%bin\;%MSYSROOT%mingw\bin\;%PATH%
sh %~dp0dscaler5build64
:EOF