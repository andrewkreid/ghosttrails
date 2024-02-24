#ifndef _IPROGRESS_H_
#define _IPROGRESS_H_

// ---------------------------------------------------------------------------
//
// Progress.h
//
// Abstract class that handles progress update and cancellation messages.
//
// ---------------------------------------------------------------------------

class IProgress {
 public:
  virtual void progressStart() = 0;
  virtual void progressNotify(int progress, int total) = 0;
  virtual void progressEnd() = 0;

  virtual bool isCancelled() = 0;
};

#endif  // _IPROGRESS_H_