/**********************************************************************
 *<
        FILE: GhostTrails.h

        DESCRIPTION:	Template Utility

        CREATED BY:

        HISTORY:

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#ifndef __GHOSTTRAILS__H
#define __GHOSTTRAILS__H

#include "max.h"

// defined in both resource.h and winuser.h - undef here
// to get rid of annoying compiler warning
#ifdef IDC_HELP
#undef IDC_HELP
#endif

#include <string>

#include "resource.h"

#include "iparamb2.h"
#include "iparamm2.h"
#include "istdplug.h"
#include "meshadj.h"

#include "ParticleState.h"

#include "FunPub.h"

// ----------------------------------------------------------------------------

extern TCHAR* GetString(int id);

extern HINSTANCE hInstance;

class GTWorkingValues;
class ITrailSource;

// ----------------------------------------------------------------------------

class GhostTrails : public Modifier, public GTInterface {
 public:
  // enums for various parameters in the pblock
  enum {
    pb_frames = 0,       // published - must not change
    pb_segments = 1,     // published - must not change
    pb_flipnormals = 2,  // published - must not change
    pb_splinesteps = 3,  // published - must not change

    pb_type,
    pb_userange,
    pb_startframe,
    pb_endframe,
    pb_segsperframe,
    pb_urepeat,

    pb_apply_tex_b,        // transient - order not important, put last
    pb_apply_tex_edit,     // transient - order not important, put last
    pb_apply_tex_vrepeat,  // transient - order not important, put last
    pb_apply_fade_b,       // transient - order not important, put last
    pb_apply_fade_mp,      // transient - order not important, put last

    pb_particle_trails_b,  // Use particle system (yes/no)
    pb_particle_node,      // Particle system node

    pb_generate_map,  // normal UV mapping
    pb_generate_age_map,

    pb_useallpf,     // use all pflow event checkbox
    pb_pfeventlist,  // list of pflow events

    pb_skippart,   // every n'th particle
    pb_startpart,  // start at particle n

    pb_maptype,  // String or Stretch Mapping
    pb_nummtls,  // Number of material IDs

    pb_segsperframe_render,     // segments/frame value when rendering.
    pb_segsperframe_render_on,  // whether the render-time segments value should
                                // be used.

    pb_splinesteps_render_DEPRECATED,  // DO NOT USE
    pb_splinesteps_render_on,  // whether the render-time spline steps value
                               // should be used.
    pb_splinesteps_render,     // spline steps value to use while rendering.

    pb_use_pflow_mtlids  // Use Particle Flow material IDs (added in
                         // 3dsmax2018).
  };

  enum GTMapType { eString = 0, eStretch };

  enum GTParamType { ghosttrails_params };
  enum GTRollupType {
    ghosttrails_map_main,
    ghosttrails_map_meshparams,
    ghosttrails_map_apply,
    ghosttrails_particle,
    NUM_ROLLUPS
  };

  // Parameter block
  IParamBlock2* pblock;     // ref 0
  IParamBlock2* pblockBAD;  // ref 0
  static IObjParam* ip;     // Access to the interface

  // Array of rollup handles. Updated by the user dlg proc after BeginEditParams
  // creates them.
  static HWND rollupHandles[NUM_ROLLUPS];

// From Animatable
#ifdef MAX2013
#ifdef MAX2022
  const TCHAR* GetObjectName(bool localized) const override;
#else
  const TCHAR* GetObjectName();
#endif
#else
  TCHAR* GetObjectName();
#endif

  virtual int RenderBegin(TimeValue t, ULONG flags);
  virtual int RenderEnd(TimeValue t);

  // From Modifier
  ChannelMask ChannelsUsed();
  ChannelMask ChannelsChanged();
  void ModifyObject(TimeValue t, ModContext& mc, ObjectState* os, INode* node);
  Class_ID InputType();
  Interval LocalValidity(TimeValue t);

  // From BaseObject
  BOOL ChangeTopology();

  CreateMouseCallBack* GetCreateMouseCallBack();
  void BeginEditParams(IObjParam* ip, ULONG flags, Animatable* prev);
  void EndEditParams(IObjParam* ip, ULONG flags, Animatable* next);

  Interval GetValidity(TimeValue t);

  // Automatic texture support
  BOOL HasUVW();
  void SetGenUVW(BOOL sw);

  // Loading/Saving
  IOResult Load(ILoad* iload);
  IOResult Save(ISave* isave);

  // From Animatable
  Class_ID ClassID();
  SClass_ID SuperClassID();
#ifdef MAX2022
  void GetClassName(MSTR& s, bool localized) const override;

#else
  void GetClassName(TSTR& s);
#endif

  RefTargetHandle Clone(RemapDir& remap);
#ifdef MAX2015
  virtual RefResult NotifyRefChanged(const Interval& changeInt,
                                     RefTargetHandle hTarget, PartID& partID,
                                     RefMessage message, BOOL propagate);
#else
  RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
                             PartID& partID, RefMessage message);
#endif

  int NumSubs();
#ifdef MAX2022
  TSTR SubAnimName(int i, BOOL isLocalized);
#else
  TSTR SubAnimName(int i);
#endif
  Animatable* SubAnim(int i);
  int NumRefs();
  RefTargetHandle GetReference(int i);
  void SetReference(int i, RefTargetHandle rtarg);

  int NumParamBlocks();
  IParamBlock2* GetParamBlock(int i);
  IParamBlock2* GetParamBlockByID(BlockID id);
  virtual void DeleteThis();

  BOOL GetShapeAtTime(INode*, PolyShape&, TimeValue, int);

  int ElimVStretch(GTWorkingValues& wv);  // new to Ghost Trails version 2.0
  double GetDistanceOverRange(
      INode* pNode, TimeValue start, TimeValue end,
      TimeValue step);  // new to Ghost Trails version 2.0

  // Constructor/Destructor
  GhostTrails();
  ~GhostTrails();
  INode* GetNode();

  std::vector<double> caElimVStretchCache;
  TCHAR _pchApplyTexFilename[MAX_PATH];

  // Is this modifier using the particle trails feature?
  BOOL IsParticleTrails();

  // Recalculate the ParticleSystemState for this modifier.
  void RecalculateParticleTrails(HWND hWnd);

  // Snapshot the particle trails to a spline
  void SnapToSpline();

  // For PFlow event filtering
  Tab<INode*> pfNodes;
  Tab<int> addPFNodes;
  void PickPFEvents(HWND hWnd);

  // Temporary table to hold particle flow events while rendering. See the
  // implementation of GhostTrails::RenderBegin() for why.
  Tab<INode*> flowEventStash;

  virtual void NotifyInputChanged(Interval changeInt, PartID partID,
                                  RefMessage message, ModContext* mc);

  // MaxScript stuff from FunPub.h
  FPInterface* GetInterface(Interface_ID id) {
    if (id == GT_FO_INTERFACE)
      return (GTInterface*)this;
    else
      return (FPInterface*)Modifier::GetInterface(id);
  }

  // Called from the MaxScript regenerate_particle_trails() method.
  virtual void RegenerateParticleTrails() {
    RecalculateParticleTrails((HWND)0);
  }

  static BOOL isDebugLogging();
  static void setDebugLogging(BOOL isLogging);

 private:
  // These methods are called from ModifyObject() to do the
  // work of building the trail mesh.
  void buildMeshVertices(GTWorkingValues&);
  void buildMeshFaces(GTWorkingValues&);
  void calculateTrailTimes(GTWorkingValues&);
  void initializeWorkingValues(GTWorkingValues&);
  void buildMeshUVVertices(GTWorkingValues& wv);
  void buildMeshUVAgeVertices(GTWorkingValues& wv);

  Point3 getNextDifferentKeyPos(ITrailSource* pTrailSource, int nTrail,
                                std::vector<TimeValue>& levelTimes,
                                int currentIdx, int step);

  BOOL isSkippedTrail(int nTrail, GTWorkingValues& wv);

  Point3 getPosAtTime(TimeValue time, INode* node);

  HWND _hwndWizards;

  // Holds the particle trails animation info (if any)
  ParticleSystemState partState;

  // Critical section to manage thread-safety
  CRITICAL_SECTION critSec;

  BOOL isRebuildingTrails;

  // Whether we're logging stuff to a log file.
  static BOOL debugLogging;

  // Keep track of whether we're in render mode or not.
  BOOL inRenderMode;
};

// ----------------------------------------------------------------------------

class GhostTrailsDlgProc : public ParamMap2UserDlgProc {
 public:
  GhostTrails* ssm;
  int rollup_id;

  GhostTrailsDlgProc(int rollupID) : ssm(0), rollup_id(rollupID) {}
  GhostTrailsDlgProc(GhostTrails* ssm_in, int rollupID)
      : ssm(ssm_in), rollup_id(rollupID) {}

#ifdef MAX64
  INT_PTR DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg,
                  WPARAM wParam, LPARAM lParam);
#else
  BOOL DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam,
               LPARAM lParam);
#endif
  void DeleteThis();

  void SetThing(ReferenceTarget* m) { ssm = (GhostTrails*)m; }

  BOOL Apply(HWND hWnd);
};

// ----------------------------------------------------------------------------

void EnableRollup_Main(HWND hWnd, IParamBlock2* pblock);
void EnableRollup_MeshParams(HWND hWnd, IParamBlock2* pblock);
void EnableRollup_Apply(HWND hWnd, IParamBlock2* pblock);
BOOL g_IsRegistrationCodeValid(const char* pchCode, DWORD* pLicType = NULL);

// ----------------------------------------------------------------------------

const int GHOSTTRAILS_MAXREGISTRATIONCODELENGTH = 100;

#define GHOSTTRAILS_CLASS_ID Class_ID(0x4e32e5b7, 0x668e2c09)

// ----------------------------------------------------------------------------

#endif  // __GHOSTTRAILS__H
