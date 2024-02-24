#include "Logging.h"
#include "GhostTrails.h"

#include <Shlobj.h>
#include <Shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")

// --------------------------------------------------------------------------

FILE* pLogFP = 0;

// --------------------------------------------------------------------------

void CloseLogFile() {
  if (pLogFP) {
    fclose(pLogFP);
    pLogFP = NULL;
  }
}

// --------------------------------------------------------------------------

void LogIt(const char* fn, const char* file, int line, const char* message) {
  if (!GhostTrails::isDebugLogging()) return;

  DWORD threadID = GetCurrentThreadId();
  DebugPrint(_T("     %s: %s: %d: %s\n"), fn, file, line, message);

  TCHAR path[MAX_PATH];
  HRESULT hr =
      SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, path);
  PathAppend(path, _T("GT.log"));

  if (!pLogFP) {
    pLogFP = _tfopen(path, _T("w"));
    if (pLogFP == 0) pLogFP = _tfopen(_T("C:\\GT.log"), _T("w"));
  }
  if (pLogFP && file) {
    SYSTEMTIME stime;

    GetLocalTime(&stime);

    WCHAR* wchFn = NULL;
    WCHAR* wchFile = NULL;
    WCHAR* wchMsg = NULL;

    if (fn && file && message) {
      int lenFn = MultiByteToWideChar(CP_ACP, 0, fn, -1, NULL, 0);
      int lenFile = MultiByteToWideChar(CP_ACP, 0, file, -1, NULL, 0);
      int lenMsg = MultiByteToWideChar(CP_ACP, 0, message, -1, NULL, 0);

      if ((lenFn > 0) && (lenFile > 0) && (lenMsg > 0)) {
        wchFn = new WCHAR[lenFn + 1];
        wchFile = new WCHAR[lenFile + 1];
        wchMsg = new WCHAR[lenMsg + 1];

        MultiByteToWideChar(CP_ACP, 0, fn, -1, (LPWSTR)wchFn, lenFn);
        MultiByteToWideChar(CP_ACP, 0, file, -1, (LPWSTR)wchFile, lenFile);
        MultiByteToWideChar(CP_ACP, 0, message, -1, (LPWSTR)wchMsg, lenMsg);

        fwprintf(pLogFP, _T("%02d:%02d:%02d.%03d [%09d] %s:%s%10d: [%s]\n"),
                 stime.wHour, stime.wMinute, stime.wSecond, stime.wMilliseconds,
                 threadID, wchFn, wchFile, line, wchMsg);
        fflush(pLogFP);

        delete[] wchFn;
        delete[] wchFile;
        delete[] wchMsg;
      }
    }
  }
}
