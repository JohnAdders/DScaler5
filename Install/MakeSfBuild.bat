@echo off
echo Dscaler make a sourceforge build batch file
echo (c) John Adcock 2004
if "%1" == "" goto usage
if "%2" == "" goto usage
cd ..\..
md DScaler5Build%1
cd DScaler5Build%1
cvs -z3 -d:ext:%2@cvs.sf.net:/cvsroot/deinterlace co DScaler5
cd DScaler5
7z a -tzip ..\DScaler5%1src.zip *.* -r
call c:\PROGRA~1\MICROS~2\VC98\bin\vcvars32.bat
cd Help
"c:\Program Files\HTML Help Workshop\hhc.exe" Dscaler5.hhp
cd ..\Filters\Decoders\MpegAudio
msdev MpegAudio.dsw /MAKE "All"
cd ..\MpegVideo
msdev MpegVideo.dsw /MAKE "All"
cd ..\..\..\Install
"c:\Program Files\Inno Setup 2\Compil32.exe" /cc DScaler5.iss
copy Output\Setup.exe ..\..\DScaler5%1.exe
cd ..\..
del /f /q /s DScaler5
rd /s /q DScaler5
echo Break if there was a problem with the above build
echo Otherwise pressing enter will send the files to the
echo incoming directory on sourceforge ready to be released
pause
echo cd incoming > ftp.txt
echo bin >> ftp.txt
echo put DScaler5%1.exe >> ftp.txt
echo put DScaler5%1src.zip >> ftp.txt
echo bye >> ftp.txt
ftp -s:ftp.txt -A upload.sf.net
del ftp.txt
goto endofbatch
:usage
echo To use this program enter the build number as parameter 1
echo and a valid sourceforge user for parameter 2
:endofbatch