# Microsoft Developer Studio Project File - Name="GhostTrails" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=GhostTrails - Win32 Hybrid  Max4
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GhostTrails.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GhostTrails.mak" CFG="GhostTrails - Win32 Hybrid  Max4"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GhostTrails - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "GhostTrails - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "GhostTrails - Win32 Hybrid" (based on "Win32 (x86) Application")
!MESSAGE "GhostTrails - Win32 Release Max4" (based on "Win32 (x86) Application")
!MESSAGE "GhostTrails - Win32 Hybrid  Max4" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/GT_Commercial", RAAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GhostTrails - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /WX /GX /O2 /I "D:\3DSMAX3_1\Maxsdk\include" /I "D:\3dsmax4.2\maxsdk\Include" /I "g:\Program Files\HTML Help Workshop\Include" /D "NDEBUG" /D "CUSTOM_MFC" /D "WIN32" /D "_WINDOWS" /D "SHELL32LIB" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib bmm.lib core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib paramblk2.lib htmlhelp.lib /nologo /base:"0x105b0000" /subsystem:windows /dll /machine:I386 /out:"c:\bdeplugins_max4\GhostTrails.dlm" /libpath:"D:\3DSMAX3_1\Maxsdk\lib" /libpath:"D:\3dsmax4.2\maxsdk\lib" /libpath:"g:\Program Files\HTML Help Workshop\lib" /release

!ELSEIF  "$(CFG)" == "GhostTrails - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /W3 /WX /Gm /GX /ZI /Od /I "D:\3dsmax4.2\maxsdk\Include" /I "g:\Program Files\HTML Help Workshop\Include" /D "_DEBUG" /D "CUSTOM_MFC" /D "WIN32" /D "_WINDOWS" /D "SHELL32LIB" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0xc09 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib bmm.lib core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib paramblk2.lib htmlhelp.lib /nologo /base:"0x105b0000" /subsystem:windows /dll /debug /machine:I386 /out:"c:\bdeplugins_max4\GhostTrails.dlm" /pdbtype:sept /libpath:"D:\3dsmax4.2\maxsdk\lib" /libpath:"g:\Program Files\HTML Help Workshop\lib"

!ELSEIF  "$(CFG)" == "GhostTrails - Win32 Hybrid"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "GhostTrails___Win32_Hybrid"
# PROP BASE Intermediate_Dir "GhostTrails___Win32_Hybrid"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Hybrid"
# PROP Intermediate_Dir "Hybrid"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MD /W3 /WX /Gm /GX /ZI /Od /I "D:\3DSMAX3_1\Maxsdk\include" /I "D:\3dsmax4.2\maxsdk\Include" /I "g:\Program Files\HTML Help Workshop\Include" /D "_DEBUG" /D "CUSTOM_MFC" /D "WIN32" /D "_WINDOWS" /D "SHELL32LIB" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0xc09 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib bmm.lib core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib paramblk2.lib htmlhelp.lib /nologo /base:"0x105b0000" /subsystem:windows /dll /debug /machine:I386 /out:"c:\bdeplugins_max4\GhostTrails.dlm" /pdbtype:sept /libpath:"D:\3DSMAX3_1\Maxsdk\lib" /libpath:"D:\3dsmax4.2\maxsdk\lib" /libpath:"g:\Program Files\HTML Help Workshop\lib"

!ELSEIF  "$(CFG)" == "GhostTrails - Win32 Release Max4"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "GhostTrails___Win32_Release_Max4"
# PROP BASE Intermediate_Dir "GhostTrails___Win32_Release_Max4"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "GhostTrails___Win32_Release_Max4"
# PROP Intermediate_Dir "GhostTrails___Win32_Release_Max4"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MD /W3 /GX /O2 /I "D:\3DSMAX3_1\Maxsdk\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /WX /GX /O2 /I "D:\3DSMAX4\Maxsdk\include" /I "D:\3dsmax4.2\maxsdk\Include" /I "g:\Program Files\HTML Help Workshop\Include" /D "NDEBUG" /D "CUSTOM_MFC" /D "WIN32" /D "_WINDOWS" /D "SHELL32LIB" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib bmm.lib core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib paramblk2.lib /nologo /base:"0x105b0000" /subsystem:windows /dll /machine:I386 /out:"D:\3DSMAX3\MyPlugins\GhostTrails.dlm" /libpath:"D:\3DSMAX3_1\Maxsdk\lib" /release
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib bmm.lib core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib paramblk2.lib htmlhelp.lib /nologo /base:"0x105b0000" /subsystem:windows /dll /machine:I386 /out:"c:\bdeplugins_max4\GhostTrails.dlm" /libpath:"D:\3DSMAX4\Maxsdk\lib" /libpath:"D:\3dsmax4.2\maxsdk\lib" /libpath:"g:\Program Files\HTML Help Workshop\lib" /release

!ELSEIF  "$(CFG)" == "GhostTrails - Win32 Hybrid  Max4"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "GhostTrails___Win32_Hybrid__Max4"
# PROP BASE Intermediate_Dir "GhostTrails___Win32_Hybrid__Max4"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Hybrid__Max4"
# PROP Intermediate_Dir "Hybrid__Max4"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MD /W3 /Gm /GX /ZI /Od /I "D:\3DSMAX3_1\Maxsdk\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /WX /Gm /GX /ZI /Od /I "D:\3dsmax4.2\maxsdk\Include" /I "g:\Program Files\HTML Help Workshop\Include" /D "CUSTOM_MFC" /D "WIN32" /D "_WINDOWS" /D "SHELL32LIB" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0xc09 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib bmm.lib core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib paramblk2.lib /nologo /base:"0x105b0000" /subsystem:windows /dll /debug /machine:I386 /out:"D:\3DSMAX3\MyPlugins\GhostTrails.dlm" /pdbtype:sept /libpath:"D:\3DSMAX3_1\Maxsdk\lib"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib bmm.lib core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib paramblk2.lib htmlhelp.lib /nologo /base:"0x105b0000" /subsystem:windows /dll /debug /machine:I386 /out:"c:\bdeplugins_max4\GhostTrails.dlm" /pdbtype:sept /libpath:"D:\3dsmax4.2\maxsdk\lib" /libpath:"g:\Program Files\HTML Help Workshop\lib"

!ENDIF 

# Begin Target

# Name "GhostTrails - Win32 Release"
# Name "GhostTrails - Win32 Debug"
# Name "GhostTrails - Win32 Hybrid"
# Name "GhostTrails - Win32 Release Max4"
# Name "GhostTrails - Win32 Hybrid  Max4"
# Begin Group "Rego Stuff"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\GhostTrailsKeyGenerator\RegCode.cpp
# End Source File
# Begin Source File

SOURCE=.\Regkey.cpp
# End Source File
# Begin Source File

SOURCE=.\Regkey.h

!IF  "$(CFG)" == "GhostTrails - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "GhostTrails - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "GhostTrails - Win32 Hybrid"

!ELSEIF  "$(CFG)" == "GhostTrails - Win32 Release Max4"

!ELSEIF  "$(CFG)" == "GhostTrails - Win32 Hybrid  Max4"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\atlassert.h
# End Source File
# Begin Source File

SOURCE=.\GhostTrails.h
# End Source File
# Begin Source File

SOURCE=.\ParticleState.h
# End Source File
# Begin Source File

SOURCE=.\ParticleStateBuilder.h
# End Source File
# Begin Source File

SOURCE=.\ScopeLock.h
# End Source File
# Begin Source File

SOURCE=.\TrailSource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\Bytegeistlogo.bmp
# End Source File
# Begin Source File

SOURCE=.\GhostTrails.rc
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\res\tire1.jpg
# End Source File
# Begin Source File

SOURCE=.\res\tire1_100x35.bmp
# End Source File
# End Group
# Begin Group "MFC_COMPAT"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\GeneralUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\GeneralUtils.h
# End Source File
# Begin Source File

SOURCE=.\mfc_compat.cpp
# End Source File
# Begin Source File

SOURCE=.\mfc_compat.h
# End Source File
# End Group
# Begin Group "Source Files"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=.\DllEntry.cpp
# End Source File
# Begin Source File

SOURCE=.\GhostTrails.cpp
# End Source File
# Begin Source File

SOURCE=.\GhostTrails.def
# End Source File
# Begin Source File

SOURCE=.\ParticleState.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleStateBuilder.cpp
# End Source File
# Begin Source File

SOURCE=.\ScopeLock.cpp
# End Source File
# Begin Source File

SOURCE=.\TrailSource.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\todo.txt
# End Source File
# End Target
# End Project
