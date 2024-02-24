// Disable deprecated warnings for strcpy etc.
#pragma warning(disable : 4996)

#include <CUSTCONT.H>
#include <STDMAT.H>
#include <decomp.h>
#include <modstack.h>
#include <plugapi.h>
#include <splshape.h>
#include <texutil.h>
#include <time.h>

#include "DlgProgress.h"
#include "GhostTrails.h"
#include "IProgress.h"
#include "Logging.h"
#include "MaxProgress.h"
#include "MaxUtils.h"
#include "ParticleStateBuilder.h"
#include "ScopeLock.h"
#include "TrailSource.h"

// ----------------------------------------------------------------------------

#define AUTH_CODE "{2ED052CA-A7F2-4ff5-8FC8-6A58EA8BA27F}"

static const int g_hKeyCount = 2;
static HKEY g_hKeys[g_hKeyCount] = {HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER};
static const char g_regpathGhostTrails[] =
    "SOFTWARE\\Bytegeist Software\\Ghost Trails";

BOOL SetWithLicensingText(HWND hDlg, int id, BOOL bLong);

// ----------------------------------------------------------------------------

// Returns the number of days left in the beta period. Each beta expires
// after a certain time.
//
//
static int gBetaPeriodDaysLeft() {
  LOGIT;

  int expiryYear = 80 + 24;  // 2004
  int expiryMonth = 3 + 2;   // June
  int expiryDay = 30;

  struct tm expiryTime;
  expiryTime.tm_sec = 0;           /* seconds after the minute - [0,59] */
  expiryTime.tm_min = 59;          /* minutes after the hour - [0,59] */
  expiryTime.tm_hour = 23;         /* hours since midnight - [0,23] */
  expiryTime.tm_mday = expiryDay;  /* day of the month - [1,31] */
  expiryTime.tm_mon = expiryMonth; /* months since January - [0,11] */
  expiryTime.tm_year = expiryYear; /* years since 1900 */
  expiryTime.tm_wday = 0;          /* days since Sunday - [0,6] */
  expiryTime.tm_yday = 0;
  expiryTime.tm_isdst = 0;

  time_t expirySecs;

  // time in seconds
  expirySecs = mktime(&expiryTime);

  // current time
  time_t curSecs = time(0);

  int daysLeft = static_cast<int>(((expirySecs - curSecs) / (60 * 60 * 24)));

  LOGIT;
  return daysLeft;
}

// ----------------------------------------------------------------------------

#ifdef MAX64
INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam,
                              LPARAM lParam)
#else
BOOL CALLBACK AboutDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
#endif
{
  switch (iMsg) {
    case WM_INITDIALOG: {
      CenterWindow(hDlg, GetCOREInterface()->GetMAXHWnd());
      SetWithLicensingText(hDlg, IDC_ABOUT_LICTEXT, TRUE);
      return TRUE;
    } break;
    case WM_CLOSE: {
      EndDialog(hDlg, TRUE);
      return TRUE;
    } break;
    case WM_COMMAND: {
      switch (LOWORD(wParam)) {
        case IDOK:
          EndDialog(hDlg, TRUE);
          break;
      }
      break;
    }
  }
  return FALSE;
}

// ----------------------------------------------------------------------------

#ifdef MAX64
INT_PTR GhostTrailsDlgProc::DlgProc(TimeValue t, IParamMap2* map, HWND hWnd,
                                    UINT msg, WPARAM wParam, LPARAM lParam)
#else
BOOL GhostTrailsDlgProc::DlgProc(TimeValue t, IParamMap2* map, HWND hWnd,
                                 UINT msg, WPARAM wParam, LPARAM lParam)
#endif
{
  LOGIT;
  int id = LOWORD(wParam);
  int blah = HIWORD(wParam);
  switch (msg) {
    case WM_COMMAND:
      switch (id) {
        case IDC_ABOUT: {
          DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUTDLG), GetActiveWindow(),
                    AboutDlgProc);
          EnableRollup_Main(
              GhostTrails::rollupHandles[GhostTrails::ghosttrails_map_main],
              ssm->pblock);
          EnableRollup_MeshParams(GhostTrails::rollupHandles
                                      [GhostTrails::ghosttrails_map_meshparams],
                                  ssm->pblock);
          EnableRollup_Apply(
              GhostTrails::rollupHandles[GhostTrails::ghosttrails_map_apply],
              ssm->pblock);

          SetWithLicensingText(hWnd, IDC_MAIN_LICTEXT, FALSE);
          SetWindowText(GetDlgItem(hWnd, IDC_APPLY_TEX_EDIT),
                        ssm->_pchApplyTexFilename);
        } break;

        case IDC_PARTICLE_GENERATE: {
          // Recalculate the particle state.
          if (ssm && ssm->IsParticleTrails()) {
            ssm->RecalculateParticleTrails(
                GetDlgItem(hWnd, IDC_PARTICLE_GENERATE));
          }
        } break;

        case IDC_PATH_SNAPSHOT: {
          // Recalculate the particle state.
          if (ssm && ssm->IsParticleTrails()) {
            ssm->SnapToSpline();
          }
        } break;

        case IDC_ADDPF_EVENT_BUTTON: {
          ssm->PickPFEvents(hWnd);
        } break;

        case IDC_MAIN_TYPE_MOVING:
        case IDC_MAIN_TYPE_ANCHORED:
        case IDC_MAIN_USERANGE: {
          EnableRollup_Main(
              GhostTrails::rollupHandles[GhostTrails::ghosttrails_map_main],
              ssm->pblock);
          break;
        }
        case IDC_APPLY_TEX_EDIT:
          if (HIWORD(wParam) == EN_SETFOCUS) {
            DisableAccelerators();
          } else if (HIWORD(wParam) == EN_KILLFOCUS) {
            EnableAccelerators();
          }
        case IDC_APPLY_TEX_B:
        case IDC_APPLY_FADE_B: {
          EnableRollup_Apply(
              GhostTrails::rollupHandles[GhostTrails::ghosttrails_map_apply],
              ssm->pblock);
          break;
        }
        case IDC_APPLY_APPLY: {
          // Create a material for generating a specific effect, and apply it to
          // this modifier node
          Apply(hWnd);
          break;
        } break;
        case IDC_ENABLE_LOGGING: {
          LRESULT retval = SendMessage(GetDlgItem(hWnd, IDC_ENABLE_LOGGING),
                                       BM_GETCHECK, 0, 0);
          if (retval == BST_CHECKED) {
            MessageBox(hWnd, _T("Logging to \"My Documents\\GT.log\""),
                       _T("Logging Enabled!"), MB_OK);
            GhostTrails::setDebugLogging(TRUE);
          } else {
            CloseLogFile();
            GhostTrails::setDebugLogging(FALSE);
          }
        } break;
        case IDC_APPLY_TEX_BROWSE: {
          BitmapInfo bi;
          BOOL bResult =
              TheManager->SelectFileInput(&bi, hWnd, _T("GhostTrails"));
          if (bResult == FALSE) return false;

          SetWindowText(GetDlgItem(hWnd, IDC_APPLY_TEX_EDIT), bi.Name());
          TCHAR* pFilename = const_cast<TCHAR*>(_tcsrchr(bi.Name(), '\\'));
          if (pFilename < bi.Name() + _tcslen(bi.Name())) {
            pFilename++;
            SetWindowText(GetDlgItem(hWnd, IDC_APPLY_TEX_TEXT), pFilename);
          }
        } break;
        case IDC_SEGMENT_IS_RENDER: {
          // Toggle whether the render params are enabled.
          EnableRollup_MeshParams(GhostTrails::rollupHandles
                                      [GhostTrails::ghosttrails_map_meshparams],
                                  ssm->pblock);
        } break;
        case IDC_SPLINESTEPS_IS_RENDER: {
          // Toggle whether the render params are enabled.
          EnableRollup_MeshParams(GhostTrails::rollupHandles
                                      [GhostTrails::ghosttrails_map_meshparams],
                                  ssm->pblock);
        } break;
      }
      break;
    case WM_INITDIALOG:
      GhostTrails::rollupHandles[rollup_id] = hWnd;
      // This winproc is being shared atleast two hWnds.
      // Atleast one of the following functions will fail
      // This is intentional - the failure is harmless
      // *if* all the IDC values in resource.h are different
      switch (rollup_id) {
        case GhostTrails::ghosttrails_map_main:
          EnableRollup_Main(
              GhostTrails::rollupHandles[GhostTrails::ghosttrails_map_main],
              ssm->pblock);
          break;
        case GhostTrails::ghosttrails_map_meshparams:
          EnableRollup_MeshParams(GhostTrails::rollupHandles
                                      [GhostTrails::ghosttrails_map_meshparams],
                                  ssm->pblock);
          break;
        case GhostTrails::ghosttrails_map_apply:
          EnableRollup_Apply(
              GhostTrails::rollupHandles[GhostTrails::ghosttrails_map_apply],
              ssm->pblock);
          break;
      }
      SetWithLicensingText(hWnd, IDC_MAIN_LICTEXT, FALSE);
      SetWindowText(GetDlgItem(hWnd, IDC_APPLY_TEX_EDIT),
                    ssm->_pchApplyTexFilename);
      SendMessage(GetDlgItem(hWnd, IDC_ENABLE_LOGGING), BM_SETCHECK,
                  GhostTrails::isDebugLogging() ? BST_CHECKED : BST_UNCHECKED,
                  0);
      break;
    case WM_DESTROY:
      GhostTrails::rollupHandles[rollup_id] = 0;
      GetWindowText(GetDlgItem(hWnd, IDC_APPLY_TEX_EDIT),
                    ssm->_pchApplyTexFilename, MAX_PATH);
      break;
  }
  LOGIT;
  return FALSE;
}

// ----------------------------------------------------------------------------

// Set the registration string in hDlg to the name of who it's registered to, or
// a generic "unregistered" message.
BOOL SetWithLicensingText(HWND hDlg, int id, BOOL bLong) {
  std::wstring sRegistered;
  if (bLong) {
    sRegistered = _T("Free Open Source Version");
  } else
    sRegistered = _T("Free Version");

  return SetWindowText(GetDlgItem(hDlg, id), sRegistered.data());
}

// ----------------------------------------------------------------------------
