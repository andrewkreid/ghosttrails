#ifndef _DLGPROGRESS_H_
#define _DLGPROGRESS_H_

class Interface;

#include "IProgress.h"
#include "ParticleState.h"

// ----------------------------------------------------------------------------

// Structure used to pass recalculation info to background thread
//
struct RecalcThreadParams {
 public:
  ParticleSystemState* pPartState;
  Interval animRange;
  IProgress* pProgress;
  INode* pParticleNode;
  Tab<INode*>* pEventList;
};

// ---------------------------------------------------------------------------
//
// DlgProgress.h
//
// Class that handles progress update and cancellation messages in Max
// by displaying a modal dialog with a progress bar and cancel button
//
// ---------------------------------------------------------------------------

class DlgProgress : public IProgress {
 public:
  enum { STATE_UNSTARTED, STATE_RUNNING, STATE_FINISHED };

  DlgProgress(ParticleSystemState*, Interval, INode*,
              Tab<INode*>* pEventList = NULL);

  int doRecalc();

  void progressStart();
  void progressNotify(int progress, int total);
  void progressEnd();

  bool isCancelled();

#ifdef MAX64
  static INT_PTR CALLBACK ProgressDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam,
                                          LPARAM lParam);
#else
  static BOOL CALLBACK ProgressDlgProc(HWND hDlg, UINT iMsg, UINT wParam,
                                       LONG lParam);
#endif

 private:
  static DlgProgress* currentInstance;

  int currentProgress;
  int progressTotal;
  int progressState;

  bool cancelled;

  RecalcThreadParams recalcParams;
};

// ----------------------------------------------------------------------------

class NullProgress : public IProgress {
 public:
  NullProgress(){};

  virtual void progressStart(){};
  virtual void progressNotify(int progress, int total){};
  virtual void progressEnd(){};

  virtual bool isCancelled() { return false; };
};

// ----------------------------------------------------------------------------

class ButtonProgress : public IProgress {
 private:
  HWND hButton;
  int curPct;

  void setButtonPctText();
  void resetButtonText();

 public:
  ButtonProgress(HWND hWnd) : hButton(hWnd), curPct(0){};

  virtual void progressStart();
  virtual void progressNotify(int progress, int total);
  virtual void progressEnd();

  virtual bool isCancelled() { return false; };
};

#endif  // _DLGPROGRESS_H_