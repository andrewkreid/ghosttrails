#include "GhostTrails.h"

#ifdef MAX6
#include <ParticleFlow/IParticleGroup.h>
#include <ParticleFlow/PFExport.h>
#include "IParticleObjectExt.h"
#endif

#include "resource.h"

// Disable deprecated warnings for strcpy etc.
#pragma warning(disable : 4996)

// ----------------------------------------------------------------------------

extern ParamBlockDesc2 ghosttrails_param_blk;

// ----------------------------------------------------------------------------

class MyEnumProc : public DependentEnumProc {
 public:
  virtual int proc(ReferenceMaker *rmaker);
  INodeTab Nodes;
};

int MyEnumProc::proc(ReferenceMaker *rmaker) {
  if (rmaker->SuperClassID() == BASENODE_CLASS_ID) {
    Nodes.Append(1, (INode **)&rmaker);
  }
  return 0;
}

// ----------------------------------------------------------------------------

static INT_PTR CALLBACK AddDlgProc(HWND hWnd, UINT msg, WPARAM wParam,
                                   LPARAM lParam) {
  GhostTrails *mod = (GhostTrails *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

  switch (msg) {
    case WM_INITDIALOG: {
      mod = (GhostTrails *)lParam;
      SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);

      for (int i = 0; i < mod->pfNodes.Count(); i++) {
#ifdef MAX2013
        const MCHAR *name;

        name = mod->pfNodes[i]->GetName();

        if (name) {
          MCHAR title[200];

          _tcscpy(title, name);

          SendMessage(GetDlgItem(hWnd, IDC_EVENTLIST), LB_ADDSTRING, 0,
                      (LPARAM)(MCHAR *)title);
        }
#else
        TCHAR *name;

        name = mod->pfNodes[i]->GetName();

        if (name) {
          TCHAR title[200];

          _tcscpy(title, name);

          SendMessage(GetDlgItem(hWnd, IDC_EVENTLIST), LB_ADDSTRING, 0,
                      (LPARAM)(MCHAR *)title);
        }
#endif
      }
      break;
    }

    case WM_COMMAND:
      switch (LOWORD(wParam)) {
        case IDOK: {
          int listCt = static_cast<int>(
              SendMessage(GetDlgItem(hWnd, IDC_EVENTLIST), LB_GETCOUNT, 0, 0));
          int selCt = static_cast<int>(SendMessage(
              GetDlgItem(hWnd, IDC_EVENTLIST), LB_GETSELCOUNT, 0, 0));
          int *selList = new int[selCt];

          SendMessage(GetDlgItem(hWnd, IDC_EVENTLIST), LB_GETSELITEMS,
                      (WPARAM)selCt, (LPARAM)selList);
          mod->addPFNodes.SetCount(selCt);
          for (int i = 0; i < selCt; i++) {
            mod->addPFNodes[i] = selList[i];
          }

          delete[] selList;

          EndDialog(hWnd, 1);
          break;
        }
        case IDCANCEL:
          mod->pfNodes.ZeroCount();
          EndDialog(hWnd, 0);
          break;
      }
      break;

    case WM_CLOSE:
      mod->pfNodes.ZeroCount();
      EndDialog(hWnd, 0);
      break;

    default:
      return FALSE;
  }
  return TRUE;
}

// ----------------------------------------------------------------------------

void GhostTrails::PickPFEvents(HWND hWnd) {
#ifdef MAX6

  Tab<INode *> pfEvents;
  int numberOfNodes = 1;
  ;

  TimeValue t = GetCOREInterface()->GetTime();

  pfNodes.ZeroCount();
  addPFNodes.ZeroCount();

  for (int i = 0; i < numberOfNodes; i++) {
    INode *node;
    Interval ivalid;

    pblock->GetValue(pb_particle_node, t, node, ivalid, i);

    if (node) {
      ObjectState tos = node->EvalWorldState(t, TRUE);

      if (tos.obj->IsParticleSystem()) {
        IParticleObjectExt *epobj = (IParticleObjectExt *)tos.obj->GetInterface(
            PARTICLEOBJECTEXT_INTERFACE);

        if (epobj) {
          MyEnumProc dep;
#ifdef MAX9
          tos.obj->DoEnumDependents(&dep);
#else
          tos.obj->EnumDependents(&dep);
#endif
          for (int i = 0; i < dep.Nodes.Count(); i++) {
            Interval valid;
            INode *node = dep.Nodes[i];

            Object *obj = node->GetObjectRef();

            if ((obj) && (obj->GetInterface(PARTICLEGROUP_INTERFACE))) {
              pfNodes.Append(1, &node);
            }
          }
        }
      }
    }
  }
  if (pfNodes.Count() > 0) {
    int iret = static_cast<int>(DialogBoxParam(hInstance,
                                               MAKEINTRESOURCE(IDD_ADD_DIALOG),
                                               hWnd, AddDlgProc, (LPARAM)this));
    if ((iret) && (addPFNodes.Count() > 0)) {
      theHold.Begin();
      for (int i = 0; i < addPFNodes.Count(); i++) {
        int index = addPFNodes[i];
        INode *node = pfNodes[index];
        pblock->Append(pb_pfeventlist, 1, &node);
      }
      theHold.Accept(GetString(IDS_ADDEVENTS));

      ghosttrails_param_blk.InvalidateUI();
    }
  }
#endif
}
