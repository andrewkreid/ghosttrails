// ---------------------------------------------------------------------------
//
// DlgProgress.h
//
// Class that handles progress update and cancellation messages in Max
// by displaying a modal dialog with a progress bar and cancel button
//
// ---------------------------------------------------------------------------

#include "Max.h"

#undef IDC_HELP
#include "resource.h"

#include "DlgProgress.h"
#include "IProgress.h"
#include "Logging.h"
#include "ParticleState.h"
#include "ParticleStateBuilder.h"

// ---------------------------------------------------------------------------

// Function to recalculate the particle trails in a
// new thread
//
DWORD WINAPI RecalcThreadFunc(LPVOID lpParam);

// ---------------------------------------------------------------------------

DlgProgress* DlgProgress::currentInstance = NULL;

// ---------------------------------------------------------------------------

DlgProgress::DlgProgress(ParticleSystemState* pPartState, Interval animRange,
                         INode* pParticleNode, Tab<INode*>* pEventList) {
  recalcParams.pPartState = pPartState;
  recalcParams.animRange = animRange;
  recalcParams.pParticleNode = pParticleNode;
  recalcParams.pProgress = this;
  recalcParams.pEventList = pEventList;

  currentProgress = 0;
  progressTotal = 100;
  progressState = STATE_UNSTARTED;
  cancelled = false;
}

// ---------------------------------------------------------------------------

BOOL DlgProgress::doRecalc() {
  currentInstance = this;

  cancelled = false;

  return static_cast<int>(DialogBox(hInstance, MAKEINTRESOURCE(IDD_PROGRESSDLG),
                                    GetActiveWindow(), ProgressDlgProc));
}

// ----------------------------------------------------------------------------

void DlgProgress::progressStart() {
  currentProgress = 0;
  progressState = STATE_RUNNING;
}

// ----------------------------------------------------------------------------

void DlgProgress::progressNotify(int progress, int total) {
  currentProgress = progress;
  progressTotal = total;
  progressState = STATE_RUNNING;
}

// ----------------------------------------------------------------------------

void DlgProgress::progressEnd() { progressState = STATE_FINISHED; }

// ----------------------------------------------------------------------------

bool DlgProgress::isCancelled() { return cancelled; }

// ----------------------------------------------------------------------------

// Dlg proc for the recalc trails dialog proc
//
#ifdef MAX64
INT_PTR CALLBACK DlgProgress::ProgressDlgProc(HWND hDlg, UINT iMsg,
                                              WPARAM wParam, LPARAM lParam)
#else
BOOL CALLBACK DlgProgress::ProgressDlgProc(HWND hDlg, UINT iMsg, UINT wParam,
                                           LONG lParam)
#endif
{
  static HANDLE hThread = 0;
  static const int TIMER_ID = 8535;
  static UINT_PTR timerHandle = 0;

  switch (iMsg) {
    case WM_INITDIALOG: {
      CenterWindow(hDlg, GetCOREInterface()->GetMAXHWnd());

      // Initialise the progress bar range (0-100)
      SendMessage(GetDlgItem(hDlg, IDC_PROGRESS), PBM_SETRANGE, 0,
                  MAKELPARAM(0, 100));
      SendMessage(GetDlgItem(hDlg, IDC_PROGRESS), PBM_SETSTEP, (WPARAM)1, 0);

      timerHandle = SetTimer(hDlg, TIMER_ID, 250, NULL);
      if (timerHandle == 0) {
        MessageBox(hDlg, _T("Could not create recalculation timer"),
                   _T("GhostTrails"), MB_OK | MB_ICONERROR);
        EndDialog(hDlg, FALSE);
      }

      // Create a background thread to do the actual recalculation work.
      DWORD threadID = 0;
      hThread =
          CreateThread(NULL, 0, RecalcThreadFunc,
                       (LPVOID)(&currentInstance->recalcParams), 0, &threadID);
      if (!hThread) {
        MessageBox(hDlg, _T("Could not create recalculation thread"),
                   _T("GhostTrails"), MB_OK | MB_ICONERROR);
        EndDialog(hDlg, FALSE);
      }

      return TRUE;
    } break;

    case WM_CLOSE: {
      currentInstance->cancelled = true;
      KillTimer(hDlg, timerHandle);
      WaitForSingleObject(hThread, 10000);
      EndDialog(hDlg, FALSE);
      return TRUE;
    } break;

    case WM_COMMAND: {
      switch (LOWORD(wParam)) {
        case IDCANCEL:
          currentInstance->cancelled = true;
          KillTimer(hDlg, timerHandle);
          WaitForSingleObject(hThread, 10000);
          EndDialog(hDlg, FALSE);
          break;
      }
      break;
    }

    case WM_TIMER:

      switch (wParam) {
        case TIMER_ID: {
          // update the progress bar
          int percentageVal =
              (int)((float)currentInstance->currentProgress /
                    (float)currentInstance->progressTotal * 100.0f);

          SendMessage(GetDlgItem(hDlg, IDC_PROGRESS), PBM_SETPOS,
                      (WPARAM)percentageVal, 0);

          // Have we finished?
          if (currentInstance->progressState == STATE_FINISHED) {
            KillTimer(hDlg, timerHandle);
            WaitForSingleObject(hThread, 10000);
            EndDialog(hDlg, TRUE);
          }
        }
          return 0;
      }
  }
  return FALSE;
}

// ----------------------------------------------------------------------------

// Function to recalculate the particle trails in a
// new thread
//
DWORD WINAPI RecalcThreadFunc(LPVOID lpParam) {
  RecalcThreadParams* pParams = (RecalcThreadParams*)lpParam;

  LOGIT;
  if (pParams) {
    LOGIT;
    ParticleStateBuilder::buildState(
        *pParams->pPartState, pParams->pParticleNode,
        pParams->animRange.Start(), pParams->animRange.End(),
        GetTicksPerFrame(), pParams->pProgress, pParams->pEventList);

    LOGIT;
    return TRUE;
  } else {
    LOGIT;
    return FALSE;
  }
}

// ----------------------------------------------------------------------------

void ButtonProgress::progressStart() {
  curPct = 0;
  setButtonPctText();
}

void ButtonProgress::progressNotify(int progress, int total) {
  int newPct = static_cast<int>(100.0f * static_cast<float>(progress) /
                                static_cast<float>(total));
  if (newPct != curPct) {
    curPct = newPct;
    setButtonPctText();
  }
}

void ButtonProgress::progressEnd() { resetButtonText(); }

void ButtonProgress::setButtonPctText() {
  if (hButton != 0) {
    TCHAR buf[40];
    _stprintf(buf, _T("%d%%"), curPct);
    SetWindowText(hButton, buf);
    InvalidateRect(hButton, NULL, TRUE);
    UpdateWindow(hButton);
  }
}

void ButtonProgress::resetButtonText() {
  if (hButton != 0) {
    SetWindowText(hButton, _T("Regenerate Trails..."));
  }
}

// ----------------------------------------------------------------------------
