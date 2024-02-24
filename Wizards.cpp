/**********************************************************************
 *<
        FILE: GhostTrails.cpp

        DESCRIPTION:	Appwizard generated plugin

        CREATED BY:

        HISTORY:

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#include "GhostTrails.h"
/*
#include "mfc_compat.h"
#include "modstack.h"
#include "plugapi.h"
#include "splshape.h"
#include "texutil.h"
*/

// ****************************************************************************

// constants
#define NUM_PAGES 6
#define MAX_BUF 5000
#define MAX_LINE 512

// typedefs
typedef struct tagREVIEWINFO {
  HINSTANCE hInst;  // current instance
  int iBmpTire;
  int iTeamWork;
  int iReliability;
  int iGoals;
  int iAdaptation;
  TCHAR pszName[MAX_PATH];
  TCHAR pszTitle[MAX_PATH];
  TCHAR pszProject[MAX_PATH];
  TCHAR pszDepartment[MAX_PATH];
  BOOL bWizardFinishedOK;
} REVIEWINFO;

// ****************************************************************************

//
//  FUNCTION: About(HWND, UINT, UINT, LONG)
//
//  PURPOSE:  Processes messages for the "About" dialog box
//
//  MESSAGES:
//
//	WM_INITDIALOG - initalizes the dialog box
//	WM_COMMAND - processes the input
//
BOOL APIENTRY About(HWND hDlg, UINT message, UINT wParam, LONG lParam) {
  switch (message) {
    case WM_INITDIALOG:
      return TRUE;

    case WM_COMMAND:
      if (LOWORD(wParam) == IDOK) {
        EndDialog(hDlg, TRUE);
        return TRUE;
      }
      break;

    default:
      return FALSE;
  }
  return FALSE;
}

//
//  FUNCTION: YourInfo(HWND, UINT, UINT, LONG)
//
//  PURPOSE:  Processes messages for "Your Information" page
//
//  MESSAGES:
//
//	WM_INITDIALOG - intializes the page
//	WM_NOTIFY - processes the notifications sent to the page
//
BOOL APIENTRY YourInfo(HWND hDlg, UINT message, UINT wParam, LONG lParam) {
  static REVIEWINFO* pResults;
  switch (message) {
    case WM_INITDIALOG:
      pResults = (REVIEWINFO*)((PROPSHEETPAGE*)lParam)->lParam;
      lstrcpy(pResults->pszName, TEXT(""));
      lstrcpy(pResults->pszTitle, TEXT(""));
      lstrcpy(pResults->pszProject, TEXT(""));
      {
        lstrcpy(pResults->pszDepartment, TEXT(""));
        PROPSHEETPAGE* pProp = (PROPSHEETPAGE*)lParam;
      }

      break;

    case WM_NOTIFY:
      switch (((NMHDR FAR*)lParam)->code) {
        case PSN_KILLACTIVE:
          SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
          return 1;
          break;

        case PSN_RESET:
          // reset to the original values
          lstrcpy(pResults->pszName, TEXT(""));
          lstrcpy(pResults->pszTitle, TEXT(""));
          lstrcpy(pResults->pszProject, TEXT(""));
          lstrcpy(pResults->pszDepartment, TEXT(""));
          SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
          break;

        case PSN_SETACTIVE:
          PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT);
          SendMessage(GetDlgItem(hDlg, 0x3024), BM_SETSTYLE,
                      (WPARAM)BS_PUSHBUTTON, MAKELONG(FALSE, 0));
          // SendMessage(GetParent(hDlg), DM_SETDEFID, (WPARAM)IDC_BUTTON1, 0);
          SendMessage(GetDlgItem(hDlg, IDE_NAME), WM_SETTEXT, 0,
                      (LPARAM)pResults->pszName);
          SendMessage(GetDlgItem(hDlg, IDE_TITLE), WM_SETTEXT, 0,
                      (LPARAM)pResults->pszTitle);
          SendMessage(GetDlgItem(hDlg, IDE_PROJECT), WM_SETTEXT, 0,
                      (LPARAM)pResults->pszProject);
          SendMessage(GetDlgItem(hDlg, IDE_DEPARTMENT), WM_SETTEXT, 0,
                      (LPARAM)pResults->pszDepartment);
          break;

        case PSN_WIZNEXT:
          // the Next button was pressed
          SendDlgItemMessage(hDlg, IDE_NAME, WM_GETTEXT, (WPARAM)MAX_PATH,
                             (LPARAM)pResults->pszName);
          SendDlgItemMessage(hDlg, IDE_TITLE, WM_GETTEXT, (WPARAM)MAX_PATH,
                             (LPARAM)pResults->pszTitle);
          SendDlgItemMessage(hDlg, IDE_PROJECT, WM_GETTEXT, (WPARAM)MAX_PATH,
                             (LPARAM)pResults->pszProject);
          SendDlgItemMessage(hDlg, IDE_DEPARTMENT, WM_GETTEXT, (WPARAM)MAX_PATH,
                             (LPARAM)pResults->pszDepartment);
          break;
        case PSN_WIZFINISH:
          pResults->bWizardFinishedOK = TRUE;
          break;
        default:
          return FALSE;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}

//
//  FUNCTION: WorkHabits(HWND, UINT, UINT, LONG)
//
//  PURPOSE:  Processes messages for "Work Habits" page
//
//  MESSAGES:
//
//	WM_INITDIALOG - intializes the page
//	WM_NOTIFY - processes the notifications sent to the page
//	WM_COMMAND - saves the id of the choice selected
//
BOOL APIENTRY procSkidPic(HWND hDlg, UINT message, UINT wParam, LONG lParam) {
  static REVIEWINFO* pResults;

  switch (message) {
    case WM_INITDIALOG:
      pResults = (REVIEWINFO*)((PROPSHEETPAGE*)lParam)->lParam;
      pResults->iBmpTire = 0;
      break;

    case WM_COMMAND:
      if (HIWORD(wParam) == BN_CLICKED) {
        pResults->iBmpTire = LOWORD(wParam);
        CheckRadioButton(hDlg, IDC_WORKHAB1, IDC_WORKHAB4, LOWORD(wParam));
      }
      break;

    case WM_NOTIFY:
      switch (((NMHDR FAR*)lParam)->code) {
        case PSN_KILLACTIVE:
          SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
          return 1;
          break;

        case PSN_RESET:
          // rest to the original values
          pResults->iBmpTire = 0;
          SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
          break;

        case PSN_SETACTIVE:
          PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_FINISH);
          if (pResults->iBmpTire)
            SendMessage(GetDlgItem(hDlg, pResults->iBmpTire), BM_SETCHECK, 1,
                        0L);
          break;

        case PSN_WIZNEXT:
          // get the selected radio button
          break;

        case PSN_WIZFINISH:
          pResults->bWizardFinishedOK = TRUE;
          break;

        default:
          return FALSE;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}

//
//  FUNCTION: TeamWork(HWND, UINT, UINT, LONG)
//
//  PURPOSE:  Processes messages for "Team Work" page
//
//  MESSAGES:
//
//	WM_INITDIALOG - intializes the page
//	WM_NOTIFY - processes the notifications sent to the page
//	WM_COMMAND - saves the id of the choice selected
//
//
BOOL APIENTRY TeamWork(HWND hDlg, UINT message, UINT wParam, LONG lParam) {
  static REVIEWINFO* pResults;

  switch (message) {
    case WM_INITDIALOG:
      pResults = (REVIEWINFO*)((PROPSHEETPAGE*)lParam)->lParam;
      pResults->iTeamWork = 0;
      break;

    case WM_COMMAND:
      if (HIWORD(wParam) == BN_CLICKED) {
        pResults->iTeamWork = LOWORD(wParam);
        CheckRadioButton(hDlg, IDC_TEAMWORK1, IDC_TEAMWORK4, LOWORD(wParam));
      }
      break;
    case WM_NOTIFY:
      switch (((NMHDR FAR*)lParam)->code) {
        case PSN_KILLACTIVE:
          SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
          return 1;
          break;

        case PSN_RESET:
          // rest to the original values
          pResults->iTeamWork = 0;
          SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
          break;

        case PSN_SETACTIVE:
          if (pResults->iTeamWork)
            SendMessage(GetDlgItem(hDlg, pResults->iTeamWork), BM_SETCHECK, 1,
                        0L);
          PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
          break;

        case PSN_WIZBACK:
          break;

        case PSN_WIZNEXT:
          break;

        case PSN_WIZFINISH:
          pResults->bWizardFinishedOK = TRUE;
          break;

        default:
          return FALSE;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}
//
//  FUNCTION: Reliability(HWND, UINT, UINT, LONG)
//
//  PURPOSE:  Processes messages for "Reliability" page
//
//  MESSAGES:
//
//	WM_INITDIALOG - intializes the page
//	WM_NOTIFY - processes the notifications sent to the page
//	WM_COMMAND - saves the id of the choice selected
//
BOOL APIENTRY Reliability(HWND hDlg, UINT message, UINT wParam, LONG lParam) {
  static REVIEWINFO* pResults;
  switch (message) {
    case WM_INITDIALOG:
      pResults = (REVIEWINFO*)((PROPSHEETPAGE*)lParam)->lParam;
      pResults->iReliability = 0;
      break;

    case WM_COMMAND:
      if (HIWORD(wParam) == BN_CLICKED) {
        pResults->iReliability = LOWORD(wParam);
        CheckRadioButton(hDlg, IDC_RELIABILITY1, IDC_RELIABILITY4,
                         LOWORD(wParam));
      }
      break;

    case WM_NOTIFY:
      switch (((NMHDR FAR*)lParam)->code) {
        case PSN_KILLACTIVE:
          SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
          return 1;
          break;

        case PSN_RESET:
          // rest to the original values
          pResults->iReliability = 0;
          SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
          break;

        case PSN_SETACTIVE:
          if (pResults->iReliability)
            SendMessage(GetDlgItem(hDlg, pResults->iReliability), BM_SETCHECK,
                        1, 0L);
          PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
          break;

        case PSN_WIZBACK:
          break;

        case PSN_WIZNEXT:
          break;

        case PSN_WIZFINISH:
          pResults->bWizardFinishedOK = TRUE;
          break;

        default:
          return FALSE;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}
//
//  FUNCTION: Goals(HWND, UINT, UINT, LONG)
//
//  PURPOSE:  Processes messages for "Goal Attainment" page
//
//  MESSAGES:
//
//	WM_INITDIALOG - intializes the page
//	WM_NOTIFY - processes the notifications sent to the page
//	WM_COMMAND - saves the id of the choice selected
//
BOOL APIENTRY Goals(HWND hDlg, UINT message, UINT wParam, LONG lParam) {
  static REVIEWINFO* pResults;
  switch (message) {
    case WM_INITDIALOG:
      pResults = (REVIEWINFO*)((PROPSHEETPAGE*)lParam)->lParam;
      pResults->iGoals = 0;
      break;

    case WM_COMMAND:
      if (HIWORD(wParam) == BN_CLICKED) {
        pResults->iGoals = LOWORD(wParam);
        CheckRadioButton(hDlg, IDC_GOALS1, IDC_GOALS4, LOWORD(wParam));
      }
      break;

    case WM_NOTIFY:
      switch (((NMHDR FAR*)lParam)->code) {
        case PSN_KILLACTIVE:
          SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
          return 1;
          break;

        case PSN_RESET:
          // rest to the original values
          pResults->iGoals = 0;
          SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
          break;

        case PSN_SETACTIVE:
          if (pResults->iGoals)
            SendMessage(GetDlgItem(hDlg, pResults->iGoals), BM_SETCHECK, 1, 0L);
          PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
          break;

        case PSN_WIZBACK:
          break;

        case PSN_WIZNEXT:
          break;

        case PSN_WIZFINISH:
          pResults->bWizardFinishedOK = TRUE;
          break;
        default:
          return FALSE;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}
//
//  FUNCTION: Adaptation(HWND, UINT, UINT, LONG)
//
//  PURPOSE:  Processes messages for "Addaptability to Change" page
//
//  MESSAGES:
//
//	WM_INITDIALOG - intializes the page
//	WM_NOTIFY - processes the notifications sent to the page
//	WM_COMMAND - saves the id of the choice selected
//
//
BOOL APIENTRY Adaptation(HWND hDlg, UINT message, UINT wParam, LONG lParam) {
  static REVIEWINFO* pResults;
  switch (message) {
    case WM_INITDIALOG:
      pResults = (REVIEWINFO*)((PROPSHEETPAGE*)lParam)->lParam;
      pResults->iAdaptation = 0;
      break;

    case WM_COMMAND:
      if (HIWORD(wParam) == BN_CLICKED) {
        pResults->iAdaptation = LOWORD(wParam);
        CheckRadioButton(hDlg, IDC_ADAPTATION1, IDC_ADAPTATION4,
                         LOWORD(wParam));
      }
      break;

    case WM_NOTIFY:
      switch (((NMHDR FAR*)lParam)->code) {
        case PSN_KILLACTIVE:
          SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
          return 1;
          break;

        case PSN_RESET:
          // rest to the original values
          pResults->iAdaptation = 0;
          SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
          break;

        case PSN_SETACTIVE:
          if (pResults->iAdaptation)
            SendMessage(GetDlgItem(hDlg, pResults->iAdaptation), BM_SETCHECK, 1,
                        0L);
          PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_FINISH);
          break;

        case PSN_WIZBACK:
          break;

        case PSN_WIZFINISH:
          pResults->bWizardFinishedOK = TRUE;
          break;

        default:
          return FALSE;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}

//
//
//  FUNCTION: FillInPropertyPage(PROPSHEETPAGE *, int, LPSTR, LPFN)
//
//  PURPOSE: Fills in the given PROPSHEETPAGE structure
//
//  COMMENTS:
//
//      This function fills in a PROPSHEETPAGE structure with the
//      information the system needs to create the page.
//
void FillInPropertyPage(PROPSHEETPAGE* psp, REVIEWINFO* pResults, int idDlg,
                        LPSTR pszProc, DLGPROC pfnDlgProc) {
  psp->dwSize = sizeof(PROPSHEETPAGE);
  psp->dwFlags = 0;
  psp->hInstance = pResults->hInst;
  psp->pszTemplate = MAKEINTRESOURCE(idDlg);
  psp->pszIcon = NULL;
  psp->pfnDlgProc = pfnDlgProc;
  psp->pszTitle = pszProc;
  psp->lParam = reinterpret_cast<LPARAM>(pResults);
}

//
//
//    FUNCTION: CreateWizard(HWND)
//
//    PURPOSE: Create the Wizard control.
//
//   COMMENTS:
//
//      This function creates the wizard property sheet.
//
int CreateWizard(HWND hwndOwner, HINSTANCE hInst, REVIEWINFO* pResults) {
  PROPSHEETPAGE psp[1];
  PROPSHEETHEADER psh;

  int nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
  for (int i = 0; i < nPages; i++) {
    switch (i) {
      case 0:
        FillInPropertyPage(&psp[0], pResults, IDD_WIZA_2SKIDPIC,
                           TEXT("Choose Picture"), procSkidPic);
        break;
      case 1:
        FillInPropertyPage(&psp[1], pResults, IDD_WIZA_1INFO,
                           TEXT("Your Information"), YourInfo);
        break;
      case 2:
        FillInPropertyPage(&psp[2], pResults, IDD_WIZA_3TEAMWORK,
                           TEXT("Team Work"), TeamWork);
        break;
      case 3:
        FillInPropertyPage(&psp[3], pResults, IDD_WIZA_4RELIABILITY,
                           TEXT("Reliability"), Reliability);
        break;
      case 4:
        FillInPropertyPage(&psp[4], pResults, IDD_WIZA_5GOALS,
                           TEXT("Attainment of Goals"), Goals);
        break;
      case 5:
        FillInPropertyPage(&psp[5], pResults, IDD_WIZA_6ADAPTATION,
                           TEXT("Adaptability to Change"), Adaptation);
        break;
    };
  }

  psh.dwSize = sizeof(PROPSHEETHEADER);
  psh.dwFlags = PSH_PROPSHEETPAGE | PSH_WIZARD | PSH_NOAPPLYNOW;
  psh.hwndParent = hwndOwner;
  psh.pszCaption = (LPSTR)TEXT("Review Wizard");
  psh.nPages = nPages;
  psh.nStartPage = 0;
  psh.ppsp = (LPCPROPSHEETPAGE)&psp;

  return (PropertySheet(&psh));
}

/*
void GenerateReview( HWND hDlg )
{
    TCHAR lpBuf1[MAX_LINE];  // Buffers for the lines in the review.
    TCHAR lpBuf2[MAX_LINE];
    TCHAR lpBuf3[MAX_LINE];
    TCHAR lpBuf4[MAX_LINE];
    TCHAR lpBuf5[MAX_LINE];


        wsprintf(lpReview, TEXT("Name: %s%C%C%C%CTitle: %s%C%C%C%CDepartment:
%s%C%C%C%CMain Project: %s%C%C%C%C"), pResults->pszName, 0x0d, 0x0a, 0x0d, 0x0a,
                pResults->pszTitle, 0x0d, 0x0a, 0x0d, 0x0a,
                pResults->pszDepartment, 0x0d, 0x0a, 0x0d, 0x0a,
                pResults->pszProject,0x0d, 0x0a, 0x0d, 0x0a );

        // Add a line describing work habits
        if (LoadString(pResults->hInst, pResults->iWorkHabits, lpBuf1,
sizeof(lpBuf1)) == 0) MessageBox(hDlg, TEXT("Error loading string!"), NULL,
MB_OK); else lstrcat(lpReview, lpBuf1);

        // Add a line describing team work
        if (LoadString(pResults->hInst, pResults->iTeamWork, lpBuf2,
sizeof(lpBuf2)) == 0) MessageBox(hDlg, TEXT("Error loading string!"), NULL,
MB_OK); else lstrcat(lpReview, lpBuf2);

        // Add a line describing reliability
        if (LoadString(pResults->hInst, pResults->iReliability, lpBuf3,
sizeof(lpBuf3)) == 0) MessageBox(hDlg, TEXT("Error loading string!"), NULL,
MB_OK); else lstrcat(lpReview, lpBuf3);

        // Add a line describing goals
        if (LoadString(pResults->hInst, pResults->iGoals, lpBuf4,
sizeof(lpBuf4)) == 0) MessageBox(hDlg, TEXT("Error loading string!"), NULL,
MB_OK); else lstrcat(lpReview, lpBuf4);

        // Add a line describing adaptability
        if (LoadString(pResults->hInst, pResults->iAdaptation, lpBuf5,
sizeof(lpBuf5)) == 0) MessageBox(hDlg, TEXT("Error loading string!"), NULL,
MB_OK); else lstrcat(lpReview, lpBuf5);

        
}
*/

BOOL CALLBACK WizardDlgProc(HWND hDlg, UINT message, WPARAM wParam,
                            LPARAM lParam) {
  static GhostTrails* pGhost;

  int id = LOWORD(wParam);
  switch (message) {     // Respond to the message ...
    case WM_INITDIALOG:  // Initialize the Controls here.
      pGhost = (GhostTrails*)lParam;
      return TRUE;
    case WM_DESTROY:  // Release the Controls here.
      return FALSE;
    case WM_COMMAND:  // Various messages come in this way.
      switch (id) {
        case IDC_ABOUT:
          // DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUTDLG),
          // GetActiveWindow(), AboutDlgProc);
          break;

        case IDC_WIZARD_SKID:
          REVIEWINFO rvInfo;  // a structure containing the review information
          rvInfo.hInst = hInstance;
          rvInfo.bWizardFinishedOK = FALSE;
          CreateWizard(hDlg, rvInfo.hInst, &rvInfo);
          if (rvInfo.bWizardFinishedOK == TRUE) {
            // Create New Materail
            Mtl* pDefMtl = NewDefaultStdMat();

            // We know it has no bitmap, or bmptex at this stage
            BitmapTex* pDefBmpTex = NewDefaultBitmapTex();
            if (!pDefBmpTex) {
              int retval =
                  MessageBox(hDlg, _T("Failed to create texture of bitmap"),
                             _T("Failed"), MB_OK);
              return FALSE;
            }

            // pDefBmpTex->BitmapLoadDlg();
            TCHAR buffer[MAX_PATH];
            _tcscpy(buffer, pGhost->ip->GetDir(APP_MATLIB_DIR));
            switch (rvInfo.iBmpTire) {
              case IDC_WORKHAB1:
                _tcscat(buffer, "\\GhostTrails\\tire1.jpg");
                break;
              case IDC_WORKHAB2:
                _tcscat(buffer, "\\GhostTrails\\tire1.jpg");
                break;
              case IDC_WORKHAB3:
                _tcscat(buffer, "\\GhostTrails\\tire2.jpg");
                break;
              default:
                _tcscat(buffer, "\\GhostTrails\\tire2.jpg");
                break;
            }

            // Set and validate
            pDefBmpTex->SetMapName(buffer);
            TCHAR* testName = pDefBmpTex->GetMapName();
            if (0) {  //_tcscmp(buffer, testName)!=0){
              int retval = MessageBox(hDlg, _T("Failed to set path of texture"),
                                      _T("Failed"), MB_OK);
              return FALSE;
            }

            pDefBmpTex->ReloadBitmapAndUpdate();
            Bitmap* test = pDefBmpTex->GetBitmap(TimeValue(0));

            pDefMtl->SetSubTexmap(ID_DI, pDefBmpTex);

            pGhost->GetNode()->SetMtl(pDefMtl);

            pGhost->pblock->SetValue(pb_urepeat, TimeValue(0), 2.0f);
          }
          break;
      }
      break;

    case WM_NOTIFY:  // Others this way...
      break;
    // Other cases...
    default:
      break;
  }

  return FALSE;
}
