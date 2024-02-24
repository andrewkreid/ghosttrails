#ifndef _MAXPROGRESS_H_
#define _MAXPROGRESS_H_

#include "IProgress.h"

class Interface;

// ---------------------------------------------------------------------------
//
// MaxProgress.h
//
// Class that handle progress update and cancellation messages in Max
//
// ---------------------------------------------------------------------------

class MaxProgress : public IProgress {
 public:
  MaxProgress(Interface* iface, const char* message);

  void progressStart();
  void progressNotify(int progress, int total);
  void progressEnd();

  bool isCancelled();

 private:
  Interface* ip;
  const char* msg;
};

#endif  // _MAXPROGRESS_H_