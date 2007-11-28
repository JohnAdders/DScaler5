How to build the ffmpeg codec dlls on windows
=============================================

Get a copy of the minimal gnu system for windows (mingw) set up especially for building ffmpeg from

http://prdownloads.sourceforge.net/mplayer-win32/MinGW-full-gcc-4.2.1.7z?download

Unpack the contents of this file to the root of your c drive and then run the file 

c:\MinGW\msys.bat

Navigate to the ffmpeg directory, note that you are in a unix shell so the directory seperators are 
forward slanting andthe drive is delimited differently.
For example if you have the DSCaler5 source in c:\source\DScaler5 do

cd /C/source/DScaler5/ffmpeg

the build the dlls using the command

./dscaler5build

When this has finished go to a nornmal dos box with the visual studio tools in the path and run 

dscaler5makelibs.cmd

To create the lib files for the dlls that the filters will link to.




