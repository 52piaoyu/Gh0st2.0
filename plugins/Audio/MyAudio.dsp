# Microsoft Developer Studio Project File - Name="MyAudio" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=MyAudio - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MyAudio.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MyAudio.mak" CFG="MyAudio - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MyAudio - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe
# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Release"
# PROP Intermediate_Dir "../Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MyAudio_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /GX /O2 /I "../../common/" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "_UNICODE" /FD /c
# SUBTRACT CPP /Fr /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib /nologo /dll /pdb:none /machine:I386 /out:"../Audio.dll" /ignore:4098
# Begin Target

# Name "MyAudio - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\common\Audio.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\Buffer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\ClientSocket.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\InstallService.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\Manager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\minilzo.c
# End Source File
# Begin Source File

SOURCE=.\MyAudio.cpp
# End Source File
# Begin Source File

SOURCE=.\MyAudioManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\myCrt.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\RegEditEx.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\until.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\common\Audio.h
# End Source File
# Begin Source File

SOURCE=..\..\common\Buffer.h
# End Source File
# Begin Source File

SOURCE=..\..\common\ClientSocket.h
# End Source File
# Begin Source File

SOURCE=..\..\common\InstallService.h
# End Source File
# Begin Source File

SOURCE=..\..\common\macros.h
# End Source File
# Begin Source File

SOURCE=..\..\common\Manager.h
# End Source File
# Begin Source File

SOURCE=.\MyAudioManager.h
# End Source File
# Begin Source File

SOURCE=..\..\common\RegEditEx.h
# End Source File
# Begin Source File

SOURCE=..\..\common\until.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
