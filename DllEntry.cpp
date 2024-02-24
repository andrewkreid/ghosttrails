/**********************************************************************
 *<
        FILE: DllEntry.cpp

        DESCRIPTION:Contains the Dll Entry stuff

        CREATED BY:

        HISTORY:

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/
#include "GhostTrails.h"

extern ClassDesc2* GetGhostTrailsDesc();

HINSTANCE hInstance;
int controlsInit = FALSE;

// This function is called by Windows when the DLL is loaded.  This
// function may also be called many times during time critical operations
// like rendering.  Therefore developers need to be careful what they
// do inside this function.  In the code below, note how after the DLL is
// loaded the first time only a few statements are executed.

BOOL WINAPI DllMain(HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved) {
  // Hang on to this DLL's instance handle.
  BOOL bRes = TRUE;
  hInstance = hinstDLL;

#ifndef MAX9
  if (!controlsInit) {
    controlsInit = TRUE;

    /*
    HINSTANCE hInstLibCheck = LoadLibrary(_T("MSVCP60.DLL"));
    if (!hInstLibCheck)
    {
            CString sMessage = "Ghost Trails requires the system file escape
    sequence "
                    "'MSVCP60.DLL' to be installed.\nYour system is missing this
    file.\n\nPlease " "contact your system administrator for assistance.
    Microsoft provides a " "system install for this file via
    'http://support.microsoft.com/?kbid=259403' "
                    "(Microsoft Knowledge Base Article - 259403). We apologize
    for any inconvenience.";
            


            MessageBox(GetActiveWindow(), sMessage, "Missing System File",
    MB_OK |MB_ICONERROR); bRes = FALSE;

    }
    FreeLibrary(hInstLibCheck);
    hInstLibCheck = NULL;
    


    hInstLibCheck = LoadLibrary(_T("MSVCRT.DLL"));
    if (!hInstLibCheck)
    {
            CString sMessage = "Ghost Trails requires the system file "
                    "'MSVCRT.DLL' to be installed.\nYour system is missing this
    file.\n\nPlease " "contact your system administrator for assistance.
    Microsoft provides a " "system install for this file via
    'http://support.microsoft.com/?kbid=259403' "
                    "(Microsoft Knowledge Base Article - 259403). We apologize
    for any inconvenience."; MessageBox(GetActiveWindow(), sMessage, "Missing
    System File",	MB_OK |MB_ICONERROR); bRes = FALSE;
    }
    FreeLibrary(hInstLibCheck);
    hInstLibCheck = NULL;
    */

    InitCustomControls(hInstance);  // Initialize MAX's custom controls
    InitCommonControls();           // Initialize Win95 controls
  }
#endif

  return bRes;
}

// This function returns a string that describes the DLL and where the user
// could purchase the DLL if they don't have it.
__declspec(dllexport) const TCHAR* LibDescription() {
  return GetString(IDS_LIBDESCRIPTION);
}

// This function returns the number of plug-in classes this DLL
// TODO: Must change this number when adding a new class
__declspec(dllexport) int LibNumberClasses() { return 1; }

// This function returns the number of plug-in classes this DLL
__declspec(dllexport) ClassDesc* LibClassDesc(int i) {
  switch (i) {
    case 0:
      return GetGhostTrailsDesc();
    default:
      return 0;
  }
}

// This function returns a pre-defined constant indicating the version of
// the system under which it was compiled.  It is used to allow the system
// to catch obsolete DLLs.
__declspec(dllexport) ULONG LibVersion() { return VERSION_3DSMAX; }

TCHAR* GetString(int id) {
  static TCHAR buf[256];

  if (hInstance)
    return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
  return NULL;
}
