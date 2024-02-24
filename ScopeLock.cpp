// **************************************************************************

#include <assert.h>
#include <windows.h>

#include "Max.h"

#include "ScopeLock.h"

//
// ScopeLock
//
// A class whose purpose is to provide an easy way to manage
// the acquisition and release of a CRITICAL_SECTION object
//
// The critical section is entered in the constructor and
// left in the destructor. This is useful for scope based
// synchronization. For example
//
// void f()
// {
//    ScopeLock(&CSObject);
//
//    // do some stuff
//
// }  // critical section released on block exit.
//
// **************************************************************************

ScopeLock::ScopeLock(CRITICAL_SECTION* cs) : m_pCritSec(cs) {
  assert(m_pCritSec);
  EnterCriticalSection(m_pCritSec);
  DebugPrint(_T("ScopeLock::EnterCriticalSection()\n"));
}

// **************************************************************************

ScopeLock::~ScopeLock() {
  assert(m_pCritSec);
  LeaveCriticalSection(m_pCritSec);
  DebugPrint(_T("ScopeLock::LeaveCriticalSection()\n"));
}

// **************************************************************************
