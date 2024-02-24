// ---------------------------------------------------------------------------
//
// MaxProgress.h
//
// Class that handle progress update and cancellation messages in Max
//
// ---------------------------------------------------------------------------

#include "MaxProgress.h"
#include "IProgress.h"
#include "Max.h"

// --------------------------------------------------------------------------

MaxProgress::MaxProgress(Interface* iface, const char* message) {
  ip = iface;
  msg = message;
}

// --------------------------------------------------------------------------

DWORD WINAPI dummyFn(LPVOID arg) { return (0); }

void MaxProgress::progressStart() {
  LPVOID arg = (LPVOID)0;
  ip->ProgressStart((TCHAR*)msg, TRUE, dummyFn, arg);
}

// --------------------------------------------------------------------------

void MaxProgress::progressNotify(int progress, int total) {
  int pct = (int)(100.0f * (float)progress / (float)total);
  ip->ProgressUpdate(pct % 100);
}

// --------------------------------------------------------------------------

void MaxProgress::progressEnd() { ip->ProgressEnd(); }

// --------------------------------------------------------------------------

bool MaxProgress::isCancelled() { return (ip->GetCancel() == TRUE); }

// --------------------------------------------------------------------------
