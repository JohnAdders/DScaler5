;////////////////////////////////////////////////////////////////////////////
;// $Id: DScaler5.iss,v 1.4 2005-02-26 14:00:25 adcockj Exp $
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
;// Revision 1.3  2004/07/11 15:29:23  adcockj
;// Updated install and help
;//
;// Revision 1.2  2004/04/14 16:29:26  adcockj
;// Updated instaler for help and to latest version of Innosetup
;//
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
AppPublisherURL=http://www.dscaler.net
AppSupportURL=http://www.dscaler.net/phpBB/
AppUpdatesURL=http://www.dscaler.net
DefaultDirName={pf}\DScaler5
DefaultGroupName=DScaler5
AllowNoIcons=yes
LicenseFile=gpl.rtf
AppMutex=DScaler5
;required for installing the driver on NT platforms
PrivilegesRequired=admin
DisableStartupPrompt=yes



[Messages]
BeveledLabel=DScaler 5
WizardLicense=GPL License Agreement
LicenseLabel3=Do you want to continue to install [name]? If you choose No, Setup will close.
WizardInfoBefore=Warning
InfoBeforeLabel=Please read the following important warning before continuing.
InfoBeforeClickLabel=When you are ready and happy to continue with Setup, click Next.

[Components]
Name: "main"; Description: "Main Files"; Types: full; Flags: fixed

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"; MinVersion: 4,4

[Files]
Source: "..\Release\MpegAudio.dll"; DestDir: "{app}"; Flags: regserver;
Source: "..\Release\MpegVideo.dll"; DestDir: "{app}"; Flags: regserver;
Source: "..\Release\GenDMOProp.dll"; DestDir: "{app}"; Flags: regserver;
Source: "..\Release\DScaler5.chm"; DestDir: "{app}"; Flags:;

[INI]
Filename: "{app}\DScaler.url"; Section: "InternetShortcut"; Key: "URL"; String: "http://www.dscaler.net"
Filename: "{app}\Support.url"; Section: "InternetShortcut"; Key: "URL"; String: "http://www.dscaler.net/phpBB/"

[Icons]
Name: "{group}\DScaler on the Web"; Filename: "{app}\DScaler.url"
Name: "{group}\Support Forum"; Filename: "{app}\Support.url"
Name: "{group}\DScaler 5 Help"; Filename: "{app}\DScaler5.chm"

[UninstallDelete]
Type: files; Name: "{app}\DScaler.url"
Type: files; Name: "{app}\Support.url"

