;////////////////////////////////////////////////////////////////////////////
;// $Id: DScaler5.iss,v 1.2 2004-04-14 16:29:26 adcockj Exp $
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
;// Revision 1.1  2004/04/08 19:03:28  adcockj
;// Added Install routine
;//
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
LicenseFile=gpl.rtf
AppMutex=Dscaler5
;required for installing the driver on NT platforms
PrivilegesRequired=Admin
VersionInfoVersion=0.0.1
SolidCompression=yes



[Messages]
[Messages]
BeveledLabel=DScaler
WizardLicense=GPL License Agreement
LicenseLabel=Please read the following important information before continuing.  The document explains the rights you have to this software.
LicenseLabel3=Do you want to continue to install [name]?.
LicenseAccepted=I want to &continue
LicenseNotAccepted=I &do not want to continue
WizardInfoBefore=Warning
InfoBeforeLabel=Please read the following important warning before continuing.
InfoBeforeClickLabel=When you are ready and happy to continue with Setup, click Next.

[Components]
Name: "main"; Description: "Main Files"; Types: full; Flags: fixed

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"; MinVersion: 4,4

[Files]
Source: "..\Release\MpegAudio.dll"; DestDir: "{app}"; Flags: regserver ignoreversion;
Source: "..\Release\MpegVideo.dll"; DestDir: "{app}"; Flags: regserver ignoreversion;
Source: "..\Release\GenDMOProp.dll"; DestDir: "{app}"; Flags: regserver ignoreversion;
Source: "..\Release\DScaler5.chm"; DestDir: "{app}"; Flags: ignoreversion;

[INI]
Filename: "{app}\DScaler.url"; Section: "InternetShortcut"; Key: "URL"; String: "http://www.dscaler.org"
Filename: "{app}\Support.url"; Section: "InternetShortcut"; Key: "URL"; String: "http://www.dscaler.org/phpBB/"

[Icons]
Name: "{group}\DScaler on the Web"; Filename: "{app}\DScaler.url"
Name: "{group}\Support Forum"; Filename: "{app}\Support.url"
Name: "{group}\DScaler 5 Help"; Filename: "{app}\DScaler5.chm"
Name: "{group}\Uninstall DScaler 5"; Filename: "{uninstallexe}"

[UninstallDelete]
Type: files; Name: "{app}\DScaler.url"
Type: files; Name: "{app}\Support.url"

