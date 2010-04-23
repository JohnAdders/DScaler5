@echo off
echo Analyse files looking for peak white
echo (c) John Adcock 2010
if "%~s1" == "" goto askforfile 
set flags=-f wtwfind  -pix_fmt rgb24
%~dp0ffmpeg -i "%~1" %flags% "%~dp0%~n1.rgb.csv"
echo *
echo * 
echo *
echo Analysis results in file %~dp0%~n1.rgb.csv
pause
exit /b
:askforfile
if "%1" == "" set /p input=File To Process (no quotes, spaces OK)- 
if "%input%" == "" exit /b
%0 "%input%"
