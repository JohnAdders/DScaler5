;////////////////////////////////////////////////////////////////////////////
;// $Id: DScaler5.iss,v 1.1 2004-04-08 19:03:28 adcockj Exp $
;/////////////////////////////////////////////////////////////////////////////
;// Copyright (c) 2002 John Adcock.  All rights reserved.
;/////////////////////////////////////////////////////////////////////////////
;//
;//  This file is subject to the terms of the GNU General Public License as
;//  published by the Free Software Foundation.  A copy of this license is
;//  included with this software distribution in the file COPYING.  If you
;//  do not have a copy, you may obtain a copy by writing to the Free
;//  Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
;//
;//  This software is distributed in the hope that it will be useful,
;//  but WITHOUT ANY WARRANTY; without even the implied warranty of
;//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;//  GNU General Public License for more details
;/////////////////////////////////////////////////////////////////////////////
;// CVS Log
;//
;// $Log: not supported by cvs2svn $
;/////////////////////////////////////////////////////////////////////////////
;
;  This is an InnoSetup script.
;  For more information about InnoSetup see http://www.innosetup.com

[Setup]
AppName=DScaler 5 Mpeg Decoders
AppVerName=DScaler 5 Mpeg Decoders
AppPublisherURL=http://www.dscaler.org
AppSupportURL=http://www.dscaler.org/phpBB/
AppUpdatesURL=http://www.dscaler.org
DefaultDirName={pf}\DScaler5
DefaultGroupName=DScaler5
AllowNoIcons=yes
AlwaysCreateUninstallIcon=yes
LicenseFile=gpl.rtf
AppMutex=Dscaler5
;required for installing the driver on NT platforms
AdminPrivilegesRequired=yes

[Messages]
BeveledLabel=DScaler5

[Components]
Name: "main"; Description: "Main Files"; Types: full; Flags: fixed

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"; MinVersion: 4,4

[Files]
Source: "..\Release\MpegAudio.dll"; DestDir: "{app}"; Flags: regserver; CopyMode: alwaysoverwrite;
Source: "..\Release\MpegVideo.dll"; DestDir: "{app}"; Flags: regserver; CopyMode: alwaysoverwrite;
Source: "..\Release\GenDMOProp.dll"; DestDir: "{app}"; Flags: regserver; CopyMode: alwaysoverwrite;

[INI]
Filename: "{app}\DScaler.url"; Section: "InternetShortcut"; Key: "URL"; String: "http://www.dscaler.org"
Filename: "{app}\Support.url"; Section: "InternetShortcut"; Key: "URL"; String: "http://www.dscaler.org/phpBB/"

[Icons]
Name: "{group}\DScaler on the Web"; Filename: "{app}\DScaler.url"
Name: "{group}\Support Forum"; Filename: "{app}\Support.url"

[UninstallDelete]
Type: files; Name: "{app}\DScaler.url"
Type: files; Name: "{app}\Support.url"

