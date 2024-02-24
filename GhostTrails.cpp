/**********************************************************************
 *<
        FILE: GhostTrails.cpp

        DESCRIPTION:	Appwizard generated plugin

        CREATED BY:

        $Id: GhostTrails.cpp,v 1.36 2007/07/09 09:39:24 andrew Exp $

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

// Disable deprecated warnings for strcpy etc.
#pragma warning(disable : 4996)

#include <time.h>
#include <wchar.h>

#include "GhostTrails.h"
#include "TrailSource.h"

#include "DlgProgress.h"
#include "IProgress.h"
#include "MaxProgress.h"
#include "MaxUtils.h"
#include "ParticleStateBuilder.h"
#include "ScopeLock.h"

#include <decomp.h>
#include <modstack.h>
#include <plugapi.h>
#include <splshape.h>
#include <texutil.h>
#include <CUSTCONT.H>
#include <STDMAT.H>

#include "Logging.h"

#define A_RENDER (A_PLUGIN1)
#define PBLOCK_REF 0

// ----------------------------------------------------------------------------

// The material channel ID of the particle age mapping channel.
static const int AGE_CHAN = 2;

HWND GhostTrails::rollupHandles[NUM_ROLLUPS];

// ----------------------------------------------------------------------------

void GhostTrailsDlgProc::DeleteThis() {
  LOGIT;
  DebugPrint(_T("GhostTrailsDlgProc::DeleteThis() \n"));
  // rollupHandles[rollup_id] = 0;
  delete this;
  LOGIT;
}

// ----------------------------------------------------------------------------

BOOL GhostTrails::debugLogging = FALSE;

// ----------------------------------------------------------------------------

class GhostTrailsClassDesc : public ClassDesc2 {
 public:
  DWORD _dwCookie;
  GhostTrailsClassDesc() {
    _dwCookie = NULL;
  }
  int IsPublic() { return 1; }
  void* Create(BOOL loading = FALSE) {

    return new GhostTrails();
  }
  const TCHAR* ClassName() { return GetString(IDS_CLASS_NAME); }
#ifdef MAX2022
  const TCHAR* NonLocalizedClassName() { return _T("GhostTrails"); }
#endif
  SClass_ID SuperClassID() { return OSM_CLASS_ID; }
  Class_ID ClassID() { return GHOSTTRAILS_CLASS_ID; }
  const TCHAR* Category() { return GetString(IDS_CATEGORY); }
  const TCHAR* InternalName() {
    return _T("GhostTrails");
  }  // returns fixed parsable name (scripter-visible name)
  HINSTANCE HInstance() { return hInstance; }  // returns owning module handle
  ~GhostTrailsClassDesc() {}
};

// ----------------------------------------------------------------------------

static GhostTrailsClassDesc GhostTrailsDesc;
ClassDesc2* GetGhostTrailsDesc() { return &GhostTrailsDesc; }


// --------------------------------------------------------------------------------------------------------

// -------------------------------------------
// parameter validator - class declaration
// -------------------------------------------
class ParticlePickerPBValidator : public PBValidator {
  // this validator picks only particle system objects
  BOOL Validate(PB2Value& v) {
    if (!v.r) return FALSE;

    static Object* ob;

    INode* node = (INode*)v.r;
    if (node->GetObjectRef() && GetParticleInterface(node->GetObjectRef()))
      return TRUE;
    else
      return FALSE;
  }
};

// global instance
static ParticlePickerPBValidator particleValidator;

// ----------------------------------------------------------------------------

#ifndef MAX2013
#define p_end end
#endif

// clang-format off
ParamBlockDesc2 ghosttrails_param_blk ( GhostTrails::ghosttrails_params, _T("main"),  0, &GhostTrailsDesc, 
	P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP, PBLOCK_REF, 
    // map rollups
		4, 
        GhostTrails::ghosttrails_map_main,			IDD_PANEL_MAIN,			IDS_PARAMS1, 0, 0, NULL,
        GhostTrails::ghosttrails_map_meshparams,		IDD_PANEL_MESHPARAMS,	IDS_PARAMS2, 0, 0, NULL,
		GhostTrails::ghosttrails_map_apply,			IDD_PANEL_APPLY,		IDS_PARAMS3, 0, 0, NULL,
		GhostTrails::ghosttrails_particle,			IDD_PANEL_PARTICLE,		IDS_PARAMS4, 0, 0, NULL,
		//ghosttrails_map_wizards,  IDD_PANEL_WIZARDS, IDS_PARAMS3, 0, 0, NULL,


	// START: Original Parameters
	// Cannot change these!
	GhostTrails::pb_frames, 		_T("old_frames"), 		TYPE_INT, 	P_ANIMATABLE, 	IDS_OLDFRAMES_SPIN, 
		p_default, 		3, 
		p_range, 		0, 100000, 
		p_ui, 			GhostTrails::ghosttrails_map_main,		TYPE_SPINNER,		EDITTYPE_INT, IDC_MAIN_FRAMES_EDIT,	IDC_MAIN_FRAMES_SPIN, SPIN_AUTOSCALE, 
		p_end,
	GhostTrails::pb_segments, 		_T("segments"), 			TYPE_INT, 	0, 	IDS_SEGMENTS_SPIN, 
		p_default, 		3, 
		p_range, 		1, 10000, 
		//p_ui, 			ghosttrails_map_meshparams, TYPE_SPINNER,		EDITTYPE_INT, IDC_SEGMENTS_EDIT, IDC_SEGMENTS_SPIN, SPIN_AUTOSCALE, 
		p_end,
	GhostTrails::pb_flipnormals, 	_T("flipNormals"),			TYPE_BOOL, 		0,				IDS_FLIPNORMALS,
		p_default, 		FALSE, 
		p_ui, 			GhostTrails::ghosttrails_map_meshparams, TYPE_SINGLECHEKBOX, 	IDC_FLIPNORMALS, 
		p_end, 
	GhostTrails::pb_splinesteps, 	_T("spline steps"), 		TYPE_INT, 	0, 	IDS_SPLINESTEPS_SPIN, 
		p_default, 		6, 
		p_range, 		0, 10000, 
		p_ui, 			GhostTrails::ghosttrails_map_meshparams, TYPE_SPINNER,		EDITTYPE_INT, IDC_SPLINESTEPS_EDIT, IDC_SPLINESTEPS_SPIN, SPIN_AUTOSCALE, 
		p_end,
	// END: Original Parameters.


	// Parameters
	GhostTrails::pb_type, 			_T("moving or still"),		TYPE_INT, 		0,				IDS_MOVINGORANCHORED,
		p_default, 		0, 
		p_ui, 			GhostTrails::ghosttrails_map_main,	TYPE_RADIO,			2,				IDC_MAIN_TYPE_MOVING,		IDC_MAIN_TYPE_ANCHORED,
		p_end, 
	GhostTrails::pb_userange, 		_T("pb_userange"),			TYPE_BOOL, 		0,				IDS_USERANGE,
		p_default, 		FALSE, 
		p_ui, 			GhostTrails::ghosttrails_map_main,	TYPE_SINGLECHEKBOX, 	IDC_MAIN_USERANGE, 
		p_end, 

	GhostTrails::pb_startframe, 	_T("start frame"), 		TYPE_INT, 		0, 	IDS_STARTFRAME_SPIN, 
		p_default, 		0, 
		p_range, 		0, 100000000, 
		p_ui, 			GhostTrails::ghosttrails_map_main,	TYPE_SPINNER,		EDITTYPE_TIME, IDC_MAIN_STARTFRAME_EDIT,	IDC_MAIN_STARTFRAME_SPIN, SPIN_AUTOSCALE, 
		p_end,
	GhostTrails::pb_endframe, 		_T("end frame"), 		TYPE_INT, 		0, 	IDS_ENDFRAME_SPIN, 
		p_default, 		GetTicksPerFrame(), 
		p_range, 		0, 100000000, 
		p_ui, 			GhostTrails::ghosttrails_map_main,	TYPE_SPINNER,		EDITTYPE_TIME, IDC_MAIN_ENDFRAME_EDIT,	IDC_MAIN_ENDFRAME_SPIN, SPIN_AUTOSCALE, 
		p_end,

	//Mesh Parameters
	GhostTrails::pb_segsperframe,	_T("segments per frame"), 	TYPE_FLOAT, 	 0, 	IDS_SEGSPERFRAME_SPIN, 
		p_default, 		1.0, 
		p_range, 		.0001, 10000.0, 
		p_ui, 			GhostTrails::ghosttrails_map_meshparams, TYPE_SPINNER,	EDITTYPE_POS_FLOAT, IDC_SEGSPERFRAME_EDIT, IDC_SEGSPERFRAME_SPIN, 0.1f, 
		p_end,

	GhostTrails::pb_urepeat,			_T("pb_urepeat"), 	TYPE_FLOAT, 	0, 	IDS_UREPEAT_SPIN, 
		p_default, 		1.0, 
		p_range, 		.0001, 10000.0, 
		p_ui, 			GhostTrails::ghosttrails_map_meshparams, TYPE_SPINNER,	EDITTYPE_FLOAT, IDC_UREPEAT_EDIT, IDC_UREPEAT_SPIN, 0.1f, 
		p_end,

	//Apply_Tex Parameters
	GhostTrails::pb_apply_tex_b, 	_T("pb_apply_tex_b"),			TYPE_BOOL, 		P_TRANSIENT,		IDS_APPLY_TEX_B,
		p_default, 		FALSE, 
		p_ui, 			GhostTrails::ghosttrails_map_apply,	TYPE_SINGLECHEKBOX, 	IDC_APPLY_TEX_B, 
		p_end, 

	GhostTrails::pb_apply_tex_vrepeat,_T("pb_apply_tex_vrepeat"), 	TYPE_FLOAT, 	P_TRANSIENT, 	IDS_APPLY_TEX_VREPEAT, 
		p_default, 		1.0, 
		p_range, 		.0001, 10000.0, 
		p_ui, 			GhostTrails::ghosttrails_map_apply, TYPE_SPINNER,	EDITTYPE_FLOAT, IDC_APPLY_TEX_VREPEAT_EDIT, IDC_APPLY_TEX_VREPEAT_SPIN, 1.0f, 
		p_end,

	//Apply_Fade Parameters	
	GhostTrails::pb_apply_fade_b, 	_T("pb_apply_fade_b"),			TYPE_BOOL, 		P_TRANSIENT,	IDS_APPLY_FADE_B,
		p_default, 		FALSE, 
		p_ui, 			GhostTrails::ghosttrails_map_apply,	TYPE_SINGLECHEKBOX, 	IDC_APPLY_FADE_B, 
		p_end, 

	GhostTrails::pb_apply_fade_mp, _T("pb_apply_fade_mp"), 	TYPE_FLOAT, 			P_TRANSIENT, 	IDS_APPLY_FADE_MP, 
		p_default, 		.8, 
		p_range, 		.0001, 1.0, 
		p_ui, 			GhostTrails::ghosttrails_map_apply, TYPE_SPINNER,	EDITTYPE_POS_FLOAT, IDC_APPLY_FADE_MP_EDIT, IDC_APPLY_FADE_MP_SPIN, 0.1f, 
		p_end,

	// Particle Trails On/Off
	GhostTrails::pb_particle_trails_b, 	_T("pb_particle_trails_b"),			TYPE_BOOL, 		0,		IDS_PARTICLE_B,
		p_default, 		FALSE, 
		p_ui, 			GhostTrails::ghosttrails_particle,	TYPE_SINGLECHEKBOX, 	IDC_PARTICLE_ENABLE, 
		p_end,
		
	// Particle Node reference
	GhostTrails::pb_particle_node,		_T("pb_particle_node"),				TYPE_INODE,		0,				IDS_PARTICLE_NODE,	// not animatable
		p_ui,			GhostTrails::ghosttrails_particle,	TYPE_PICKNODEBUTTON,	IDC_PARTICLE_CHOOSE,
		p_validator,	&particleValidator,
		p_prompt,		IDS_PARTICLE_NODE_PROMPT,
		p_end,

	GhostTrails::pb_generate_map, 	_T("generateMap"),			TYPE_BOOL, 		0,				IDS_GENERATE_MAP,
		p_default, 		TRUE, 
		p_ui, 			GhostTrails::ghosttrails_map_meshparams, TYPE_SINGLECHEKBOX, 	IDC_GENERATE_MAP, 
		p_end, 
	GhostTrails::pb_generate_age_map, 	_T("generateAgeMap"),			TYPE_BOOL, 		0,				IDS_GENERATE_AGE_MAP,
		p_default, 		FALSE, 
		p_ui, 			GhostTrails::ghosttrails_particle, TYPE_SINGLECHEKBOX, 	IDC_GENERATE_AGE_MAP, 
		p_end, 

	// PFlow event filtering
	GhostTrails::pb_useallpf, 	_T("useAllPFEvents"), 	TYPE_BOOL, 		0,				IDS_USEALLPFEVENTS,
	    p_default,		TRUE,
		p_ui, 			GhostTrails::ghosttrails_particle, 			TYPE_SINGLECHEKBOX, 	IDC_USEALLPF, 
		p_end, 
	GhostTrails::pb_pfeventlist,    _T("pfEventList"),  TYPE_INODE_TAB,		0,	P_AUTO_UI|P_VARIABLE_SIZE,	IDS_PFEVENTLIST,
		p_ui, 			GhostTrails::ghosttrails_particle,			TYPE_NODELISTBOX, IDC_PFLIST,0,0,IDC_PFREMOVE,
		p_end,
		
	GhostTrails::pb_skippart,			_T("pb_skippart"), 	TYPE_INT, 	0, 	IDS_SKIPPART, 
		p_default, 		1, 
		p_range, 		1, 1000, 
		p_ui, 			GhostTrails::ghosttrails_particle, TYPE_SPINNER,	EDITTYPE_INT, IDC_PARTICLE_SKIPPARTS_EDIT, IDC_PARTICLE_SKIPPARTS_SPIN, SPIN_AUTOSCALE, 
		p_end,

	GhostTrails::pb_startpart,			_T("pb_startpart"), 	TYPE_INT, 	0, 	IDS_STARTPART, 
		p_default, 		1, 
		p_range, 		1, 1000, 
		p_ui, 			GhostTrails::ghosttrails_particle, TYPE_SPINNER,	EDITTYPE_INT, IDC_PARTICLE_STARTPART_EDIT, IDC_PARTICLE_STARTPART_SPIN, SPIN_AUTOSCALE, 
		p_end,


	// String or Stretch mapping option.
	GhostTrails::pb_maptype, 			_T("maptype"),		TYPE_INT, 		0,				IDS_MAPTYPE,
		p_default, 		0, 
		p_ui, 			GhostTrails::ghosttrails_map_meshparams,	TYPE_RADIO,			2,				IDC_MAPTYPE_STRING,		IDC_MAPTYPE_STRETCH,
		p_end, 

	GhostTrails::pb_nummtls,			_T("pb_nummtls"), 	TYPE_INT, 	0, 	IDS_NUMMTLS, 
		p_default, 		1, 
		p_range, 		1, 1000, 
		p_ui, 			GhostTrails::ghosttrails_map_meshparams, TYPE_SPINNER,	EDITTYPE_INT, IDC_NUMMTLS_EDIT, IDC_NUMMTLS_SPIN, SPIN_AUTOSCALE, 
		p_end,

	// Render params for segs per frame and spline steps. Added for v 3.61

	GhostTrails::pb_segsperframe_render,	_T("segments per frame render"), 	TYPE_FLOAT, 	 0, 	IDS_SEGSPERFRAME_RENDER, 
		p_default, 		1.0, 
		p_range, 		.0001, 10000.0, 
		p_ui, 			GhostTrails::ghosttrails_map_meshparams, TYPE_SPINNER,	EDITTYPE_POS_FLOAT, IDC_SEGSPERFRAME_RENDER_EDIT, IDC_SEGSPERFRAME_RENDER_SPIN, 0.1f, 
		p_end,
	GhostTrails::pb_segsperframe_render_on, 	_T("segments_render_on"), 	TYPE_BOOL, 		0,				IDS_SEGSPERFRAME_USE_RENDER,
	    p_default,		FALSE,
		p_ui, 			GhostTrails::ghosttrails_map_meshparams, 			TYPE_SINGLECHEKBOX, 	IDC_SEGMENT_IS_RENDER, 
		p_end, 
	GhostTrails::pb_splinesteps_render,	_T("spline steps render"), 	TYPE_INT, 	 0, 	IDS_SPLINESTEPS_RENDER, 
		p_default, 		6, 
		p_range, 		0, 10000, 
		p_ui, 			GhostTrails::ghosttrails_map_meshparams, TYPE_SPINNER,	EDITTYPE_INT, IDC_SPLINESTEPS_RENDER_EDIT, IDC_SPLINESTEPS_RENDER_SPIN, SPIN_AUTOSCALE, 
		p_end,
	GhostTrails::pb_splinesteps_render_on, 	_T("splinesteps_render_on"), 	TYPE_BOOL, 		0,				IDS_SPLINESTEPS_USE_RENDER,
	    p_default,		FALSE,
		p_ui, 			GhostTrails::ghosttrails_map_meshparams, 			TYPE_SINGLECHEKBOX, 	IDC_SPLINESTEPS_IS_RENDER, 
		p_end, 

	GhostTrails::pb_use_pflow_mtlids, _T("usePFlowMtlIDs"), TYPE_BOOL, 0, IDS_USE_PFLOW_MTLIDS,
		p_default, FALSE,
		p_ui, GhostTrails::ghosttrails_particle, TYPE_SINGLECHEKBOX, IDC_USE_PFLOW_MTLIDS,
		p_end,


	p_end
);
// clang-format on

// ----------------------------------------------------------------------------

void SetAllDataDefault(IParamBlock2* pblock) {
  TimeValue t(0);

  // NOTE: Design decision. Do NOT corrupt the original GT1.0 data

  LOGIT;
  /*
pb_type,
pb_userange,
pb_startframe,
pb_endframe,
pb_segsperframe,
pb_urepeat,
  */

  // Non animatable.
  pblock->SetValue(GhostTrails::pb_userange, t, FALSE);
  pblock->SetValue(GhostTrails::pb_startframe, t, 1);
  pblock->SetValue(GhostTrails::pb_endframe, t, 100 * GetTicksPerFrame());
  pblock->SetValue(GhostTrails::pb_segsperframe, t, 1.0f);
  pblock->SetValue(GhostTrails::pb_segsperframe_render, t, 1.0f);
  pblock->SetValue(GhostTrails::pb_splinesteps_render, t, 1);
  pblock->SetValue(GhostTrails::pb_type, t, 0);
  pblock->SetValue(GhostTrails::pb_urepeat, t, 1.0f);
  pblock->SetValue(GhostTrails::pb_generate_map, t, TRUE);
  pblock->SetValue(GhostTrails::pb_generate_age_map, t, TRUE);
  pblock->SetValue(GhostTrails::pb_segsperframe_render_on, t, FALSE);
  pblock->SetValue(GhostTrails::pb_splinesteps_render_on, t, FALSE);
  LOGIT;
}

// ----------------------------------------------------------------------------

void EnableRollup_Main(HWND hWnd, IParamBlock2* pblock) {
  LOGIT;
  if (hWnd == 0) return;

  BOOL bIsAnchored = FALSE;
  pblock->GetValue(GhostTrails::pb_type, TimeValue(0), bIsAnchored, FOREVER);

  if (!bIsAnchored) {
    pblock->SetValue(GhostTrails::pb_userange, TimeValue(0), false);
  }

  BOOL bShowRange /*, bShowLag*/;
  pblock->GetValue(GhostTrails::pb_userange, TimeValue(0), bShowRange, FOREVER);
  // pblock->GetValue(GhostTrails::pb_type, TimeValue(0), bShowLag, FOREVER);
  // bShowLag = !bShowLag;

  {
    // Start Frame.
    EnableWindow(GetDlgItem(hWnd, IDC_MAIN_TEXT1), bShowRange);
    ISpinnerControl* ISpin =
        GetISpinner(GetDlgItem(hWnd, IDC_MAIN_STARTFRAME_SPIN));
    if (ISpin) ISpin->Enable(bShowRange);
  }

  {
    // End Frame.
    EnableWindow(GetDlgItem(hWnd, IDC_MAIN_TEXT2), bShowRange);
    ISpinnerControl* ISpin2 =
        GetISpinner(GetDlgItem(hWnd, IDC_MAIN_ENDFRAME_SPIN));
    if (ISpin2) ISpin2->Enable(bShowRange);
  }
  {
    // Frames to lag.
    EnableWindow(GetDlgItem(hWnd, IDC_MAIN_TEXT3), !bIsAnchored);
    ISpinnerControl* ISpin2 =
        GetISpinner(GetDlgItem(hWnd, IDC_MAIN_FRAMES_SPIN));
    if (ISpin2) ISpin2->Enable(!bIsAnchored);
  }

  {  // The things that are only greyed out no bIsRegd
    // Anchored Radio.
    EnableWindow(GetDlgItem(hWnd, IDC_MAIN_TYPE_ANCHORED), TRUE);

    // Range Check Box
    EnableWindow(GetDlgItem(hWnd, IDC_MAIN_USERANGE), bIsAnchored);
  }

  LOGIT;
}
// ----------------------------------------------------------------------------

void EnableRollup_MeshParams(HWND hWnd, IParamBlock2* pblock) {
  LOGIT;
  if (hWnd == 0) return;

  // enable because we're registered (always true now in free version).
  BOOL bEnable = TRUE;

  // Segs Per Frames.
  BOOL bUseRenderSegs = FALSE;
  pblock->GetValue(GhostTrails::pb_segsperframe_render_on, 0, bUseRenderSegs,
                   FOREVER);

  EnableWindow(GetDlgItem(hWnd, IDC_SEGSPERFRAME_VIEWPORT_LABEL), bEnable);
  EnableWindow(GetDlgItem(hWnd, IDC_SEGSPERFRAME_RENDER_LABEL),
               bEnable && bUseRenderSegs);

  EnableWindow(GetDlgItem(hWnd, IDC_SEGMENT_IS_RENDER), bEnable);
  ISpinnerControl* ISpin = GetISpinner(GetDlgItem(hWnd, IDC_SEGSPERFRAME_SPIN));
  if (ISpin) ISpin->Enable(bEnable);
  ISpinnerControl* ISpinRender =
      GetISpinner(GetDlgItem(hWnd, IDC_SEGSPERFRAME_RENDER_SPIN));
  if (ISpinRender) ISpinRender->Enable(bEnable && bUseRenderSegs);

  BOOL bUseRenderSplineSteps = FALSE;
  pblock->GetValue(GhostTrails::pb_splinesteps_render_on, 0,
                   bUseRenderSplineSteps, FOREVER);
  EnableWindow(GetDlgItem(hWnd, IDC_SPLINESTEPS_RENDER_LABEL),
               bUseRenderSplineSteps);
  ISpinRender = GetISpinner(GetDlgItem(hWnd, IDC_SPLINESTEPS_RENDER_SPIN));
  if (ISpinRender) ISpinRender->Enable(bUseRenderSplineSteps);

  // URepeat
  EnableWindow(GetDlgItem(hWnd, IDC_MESHPARAMS_TEXT3), bEnable);
  ISpinnerControl* ISpinURepeat =
      GetISpinner(GetDlgItem(hWnd, IDC_UREPEAT_SPIN));
  if (ISpinURepeat) ISpinURepeat->Enable(bEnable);

  LOGIT;
}

// ----------------------------------------------------------------------------

void EnableRollup_Apply(HWND hWnd, IParamBlock2* pblock) {
  LOGIT;
  if (hWnd == 0) return;

  TCHAR pchEditContents[200];
  BOOL bShowTex, bShowFade, bShowButton;
  pblock->GetValue(GhostTrails::pb_apply_tex_b, TimeValue(0), bShowTex,
                   FOREVER);
  pblock->GetValue(GhostTrails::pb_apply_fade_b, TimeValue(0), bShowFade,
                   FOREVER);
  GetWindowText(GetDlgItem(hWnd, IDC_APPLY_TEX_EDIT), pchEditContents, 199);
  bShowButton = bShowFade || (bShowTex && *pchEditContents);

  {
    // Button.
    EnableWindow(GetDlgItem(hWnd, IDC_APPLY_APPLY), bShowButton);
  }

  {
    // Tex.
    EnableWindow(GetDlgItem(hWnd, IDC_APPLY_TEX_EDIT), bShowTex);
    EnableWindow(GetDlgItem(hWnd, IDC_APPLY_TEX_TEXT), bShowTex);
    EnableWindow(GetDlgItem(hWnd, IDC_APPLY_TEX_TEXT2), bShowTex);
    EnableWindow(GetDlgItem(hWnd, IDC_APPLY_TEX_BROWSE), bShowTex);
    ISpinnerControl* ISpin =
        GetISpinner(GetDlgItem(hWnd, IDC_APPLY_TEX_VREPEAT_SPIN));
    if (ISpin) ISpin->Enable(bShowTex);
  }
  {
    // Fade.

    EnableWindow(GetDlgItem(hWnd, IDC_APPLY_FADE_TEXT1), bShowFade);
    ISpinnerControl* ISpin2 =
        GetISpinner(GetDlgItem(hWnd, IDC_APPLY_FADE_MP_SPIN));
    if (ISpin2) ISpin2->Enable(bShowFade);
  }

  {  // The things that are only greyed out no bIsRegd (always TRUE now).
    EnableWindow(GetDlgItem(hWnd, IDC_APPLY_TEX_B), TRUE);
    EnableWindow(GetDlgItem(hWnd, IDC_APPLY_FADE_B), TRUE);
  }

  LOGIT;
}

// ----------------------------------------------------------------------------

IObjParam* GhostTrails::ip = NULL;

//--- GhostTrails -------------------------------------------------------
// ----------------------------------------------------------------------------

GhostTrails::GhostTrails() {
  debugLogging = FALSE;
  LOGIT;
  pblock = pblockBAD = 0;
  inRenderMode = FALSE;

  InitializeCriticalSection(&critSec);
  isRebuildingTrails = FALSE;

  *_pchApplyTexFilename = 0;
  GhostTrailsDesc.MakeAutoParamBlocks(this);
  LOGIT;
}

// ----------------------------------------------------------------------------

GhostTrails::~GhostTrails() {
  LOGIT;
  DeleteAllRefsFromMe();
  DeleteCriticalSection(&critSec);
  LOGIT;
}

// ----------------------------------------------------------------------------

Interval GhostTrails::LocalValidity(TimeValue t) {
  LOGIT;

  // if being edited, return NEVER forces a cache to be built
  // after previous modifier.
  if (TestAFlag(A_MOD_BEING_EDITED)) return NEVER;
  // TODO: Return the validity interval of the modifier

  LOGIT;
  return Interval(t, t);
  //	return NEVER;
}

// ----------------------------------------------------------------------------

RefTargetHandle GhostTrails::Clone(RemapDir& remap) {
  ScopeLock scopeLock(&critSec);
  LOGIT;
  GhostTrails* newmod = new GhostTrails();
  // TODO: Add the cloning code here
  LOGIT;
  newmod->ReplaceReference(0, pblock->Clone(remap));
  LOGIT;
  return (newmod);
}

// ----------------------------------------------------------------------------

class CModifyException {};

// ----------------------------------------------------------------------------

//
// Calculate and return the following values from the GT parameters
//
// pStart -
//
//
// Returns TRUE is all values could be calculated successfully, FALSE otherwise
//
BOOL gGetStartAndEndTimes(IParamBlock2* pBlock, Interval& curTime,
                          TimeValue* pStart, TimeValue* pEnd, double* pLevels,
                          double* pTicksPerLevel, BOOL inRenderMode) {
  // LOGIT;
  if (!pBlock) return FALSE;

  // Get start/end frame usage
  BOOL bShowRange;
  pBlock->GetValue(GhostTrails::pb_userange, TimeValue(0), bShowRange, FOREVER);

  // Get frame values for start and end
  int iStartFrame = curTime.Start();
  int iEndFrame = curTime.End();

  if (bShowRange) {
    pBlock->GetValue(GhostTrails::pb_startframe, TimeValue(0), iStartFrame,
                     FOREVER);
    pBlock->GetValue(GhostTrails::pb_endframe, TimeValue(0), iEndFrame,
                     FOREVER);

    // Not saved as frames anymore (as of v3.06)
    // iStartFrame *= GetTicksPerFrame();
    // iEndFrame *= GetTicksPerFrame();
  }

  // Get ticks values for start and end
  if (iStartFrame > iEndFrame) return FALSE;

  int ticksStartFrame = iStartFrame;  //* GetTicksPerFrame();
  int ticksEndFrame = iEndFrame;      //* GetTicksPerFrame();

  // Get levels per frame
  BOOL bUseRenderSegs = FALSE;
  pBlock->GetValue(GhostTrails::pb_segsperframe_render_on, 0, bUseRenderSegs,
                   FOREVER);

  float fLevelsPerFrame = 0.0;
  if (bUseRenderSegs && inRenderMode)
    pBlock->GetValue(GhostTrails::pb_segsperframe_render, 0, fLevelsPerFrame,
                     FOREVER);
  else
    pBlock->GetValue(GhostTrails::pb_segsperframe, 0, fLevelsPerFrame, FOREVER);

  // Calculate fTicksPerLevel
  if (fLevelsPerFrame == 0.0) return FALSE;
  double fTicksPerLevel = (1.0 / fLevelsPerFrame) * GetTicksPerFrame();

  // Crop the end frames against the given time interval.
  if (curTime.End() < ticksStartFrame) return FALSE;

  if (curTime.End() < ticksEndFrame) ticksEndFrame = curTime.End();

  // Calculate how many levels.
  if (fTicksPerLevel == 0.0) return FALSE;

  double fLevels = (ticksEndFrame - ticksStartFrame) / fTicksPerLevel;

  if (pStart) *pStart = ticksStartFrame;
  if (pEnd) *pEnd = ticksEndFrame;
  if (pTicksPerLevel) *pTicksPerLevel = fTicksPerLevel;
  if (pLevels) *pLevels = fLevels;

  // LOGIT;
  return TRUE;
}

// ----------------------------------------------------------------------------

//
// GTWorkingValues is a holding pen for all the calculations, data structures
// and intermediate values involved in building the mesh in
// GhostTrails::ModifyObject(). The idea is to make it easy to pass the state
// of the calculation at any point to another method.
//
class GTWorkingValues {
 public:
  GTWorkingValues() {}

  BOOL texturing;                  // Should UVs be generated for the mesh.
  BOOL ageTexturing;               // Should the Age UV channel (2) be generated
  BOOL usePFlowMtlIDs;             // Should we take material IDs from PFlow.
  GhostTrails::GTMapType mapType;  // Stretch or String Mapping

  BOOL bIsPathAnchored;  // Trailing or anchored path.
  BOOL flipNormals;      // Are normal to be flipped.
  int nSplineSegments;   // Number of spline segments in the trail.
  float fURepeat;        // number of times the texture repeats in U
  float fVRepeat;        // number of times the texture repeats in V
  int iNumMtlIDs;        // Number of material IDs to apply to trails.
  BOOL bEasyFade;        // ??

  std::vector<TimeValue>
      caLevelTimes;       // The time (ticks) of each segment in the trail.
  int nMotionlessLevels;  // Number of levels at the end of the trail that are
                          // in the same location. This value used by stretch
                          // mapping to correct UVs

  double fOverlapPercentage;
  double fLevelsIfShowAll;
  TimeValue ticksStartTime;
  TimeValue ticksEndTime;
  TimeValue minTrailTime;  // minimum time at which we will generate a trail for
                           // this frame.

  BOOL bUsingCache;  // ???

  TriObject* tri;    // The TriObject for the trail mesh.
  PolyShape pShape;  // The shape object to use for trail construction.

  int polys;        // total number of polygons per trail level.
  int levelVerts;   // Vertexes per trail level.
  int levelFaces;   // Faces per trail level
  int levelTVerts;  // UV verts per trail level.
  int verts;        // Total number of verts in the mesh
  int faces;        // Total number of faces in the mesh.
  int tVerts;       // Total number of UV verts in the mesh.

  TimeValue t;  // The time (ticks) of mesh generation.

  ITrailSource*
      trailSource;  // The source of the trails (spline animation or particles)

  int numActiveTrails;  // The number of active (ie not stationary) trails

  INode* theNode;

  int skipParticle;   // every n'th particle...
  int startParticle;  // starting at particle m

 private:
  // don't allow object copies
  GTWorkingValues(const GTWorkingValues&);
  GTWorkingValues& operator=(const GTWorkingValues&);
};

// ----------------------------------------------------------------------------

class GhostTrailsLocalModData : public LocalModData {
 public:
  GhostTrailsLocalModData();
  virtual ~GhostTrailsLocalModData() {}

  virtual LocalModData* Clone();

  // used to store the node pointer for a modifier.
  INode* pNode;
};

// ----------------------------------------------------------------------------

GhostTrailsLocalModData::GhostTrailsLocalModData() { pNode = NULL; }

//
// Clone the object
//
LocalModData* GhostTrailsLocalModData::Clone() {
  GhostTrailsLocalModData* pClone = new GhostTrailsLocalModData();

  // force recalculation of node (as the copied modifer will probably be
  // assigned to a newly created one).
  pClone->pNode = NULL;

  return pClone;
}

// ----------------------------------------------------------------------------

Interval GhostTrails::GetValidity(TimeValue t) {
  LOGIT;
  Interval valid;
  valid.SetInstant(t);
  LOGIT;
  return valid;
}

// ----------------------------------------------------------------------------

void GhostTrails::ModifyObject(TimeValue t, ModContext& mc, ObjectState* os,
                               INode* node) {
  LOGITM("Entering ModifyObject()");

  // LOGIT;
  ScopeLock scopeLock(&critSec);

  // to hold the working values for the mesh calculation.
  GTWorkingValues wv;

  wv.t = t;

  Interval valid;
  valid.SetInstant(t);

  // DebugPrint("In ModifyObject - time %d os = %x\n", t, os);

  try {
    if (isRebuildingTrails) {
      // We don't want to redraw while the trails are rebuilding
      DebugPrint(_T("GhostTrails::ModifyObject called during trail rebuild\n"));
      LOGITM("GhostTrails::ModifyObject called during trail rebuild");
      throw CModifyException();
    }

    if (!mc.localData) {
      LOGITM("Making a new GhostTrailsLocalModData");
      mc.localData = new GhostTrailsLocalModData;  // 3dsmax is responsible for
                                                   // deleting mc.localData.
    }

    GhostTrailsLocalModData* gtLocalData =
        (GhostTrailsLocalModData*)mc.localData;

    if (!gtLocalData->pNode) {
      LOGIT;
      gtLocalData->pNode = MaxUtils::FindNodeReference(this, &mc);
      LOGIT;
    }

    if (!gtLocalData->pNode)
      throw CModifyException();  // Can't find the node for some reason (should
                                 // never happen).

    wv.theNode = gtLocalData->pNode;

    // DebugPrint("  In ModifyObject - Node is %s [%x]\n",
    // wv.theNode->GetName(), wv.theNode);

    if (os->obj->SuperClassID() != SHAPE_CLASS_ID) throw CModifyException();

    LOGIT;
    initializeWorkingValues(wv);

    LOGIT;
    calculateTrailTimes(wv);

    // Get Mesh object to set points and faces.
    Mesh& mesh = wv.tri->GetMesh();

    // Get Shape at Time.
    for (int poly = 0; poly < wv.polys; ++poly) {
      PolyLine& line = wv.pShape.lines[poly];
      if (line.numPts <= 0) continue;
      if (line.IsClosed()) {
        wv.levelTVerts++;
      }
      wv.levelVerts += line.numPts;
      wv.levelTVerts += line.numPts;
      wv.levelFaces += (line.Segments() * 2);
    }

    LOGIT;
    // work out how many of the trails need to have a mesh generated for them
    wv.numActiveTrails = 0;
    for (int nTrail = 0; nTrail < wv.trailSource->numTrails(); nTrail++) {
      if (!isSkippedTrail(nTrail, wv)) {
        wv.numActiveTrails++;
      }
    }

    LOGIT;
    // DebugPrint("Num Active Trails at %d = %d (of %d)\n", wv.t /
    // GetTicksPerFrame(), wv.numActiveTrails, wv.trailSource->numTrails());

    wv.verts = wv.numActiveTrails * wv.levelVerts * (wv.caLevelTimes.size());
    wv.tVerts = wv.numActiveTrails * wv.caLevelTimes.size() * wv.levelTVerts;
    wv.faces =
        wv.numActiveTrails * wv.levelFaces * (wv.caLevelTimes.size() - 1);

    mesh.setNumVerts(wv.verts);
    mesh.setNumFaces(wv.faces);
    if (wv.texturing) {
      mesh.setNumTVerts(wv.tVerts);
      mesh.setNumTVFaces(wv.faces);
    }

    LOGIT;
    buildMeshUVVertices(wv);

    LOGIT;
    buildMeshUVAgeVertices(wv);

    LOGIT;
    buildMeshVertices(wv);

    LOGIT;
    buildMeshFaces(wv);

    LOGIT;
    mesh.InvalidateGeomCache();
    mesh.InvalidateTopologyCache();

    LOGIT;
    // wv.tri->SetChannelValidity(ALL_CHANNELS, valid);
    wv.tri->SetChannelValidity(TOPO_CHAN_NUM, valid);
    wv.tri->SetChannelValidity(GEOM_CHAN_NUM, valid);
    wv.tri->SetChannelValidity(TEXMAP_CHAN_NUM, valid);
    wv.tri->SetChannelValidity(MTL_CHAN_NUM, valid);
    wv.tri->SetChannelValidity(SELECT_CHAN_NUM, valid);
    wv.tri->SetChannelValidity(SUBSEL_TYPE_CHAN_NUM, valid);
    wv.tri->SetChannelValidity(DISP_ATTRIB_CHAN_NUM, valid);
    wv.tri->SetChannelValidity(VERT_COLOR_CHAN_NUM, valid);

    os->obj = wv.tri;

    // VV os->obj->UpdateValidity(OBJ_CHANNELS, valid);

    // const Mesh& m = static_cast<TriObject*>(os->obj)->GetMesh();
    // DebugPrint("  In ModifyObject - MeshStats %d:%d:%d\n", m.getNumVerts(),
    // m.getNumFaces(), m.getNumTVerts());

    LOGIT;
    os->obj->UnlockObject();
    LOGIT;
  } catch (...) {
    // something went wrong - just invalidate the object and return.

    LOGIT;
    TriObject* tri = CreateNewTriObject();

    if (!tri) return;

    LOGIT;
    // Go to the effort of creating real geometry to minimise the chance that
    // 3dsmax will decide it never needs to evaluate the object again.
    tri->mesh.setNumVerts(3);
    tri->mesh.setNumFaces(1);

    tri->mesh.verts[0].x = 0.0f;
    tri->mesh.verts[0].y = 0.0f;
    tri->mesh.verts[0].z = 0.0f;
    tri->mesh.verts[1].x = 1.0f;
    tri->mesh.verts[1].y = 0.0f;
    tri->mesh.verts[1].z = 0.0f;
    tri->mesh.verts[2].x = 0.0f;
    tri->mesh.verts[2].y = 1.0f;
    tri->mesh.verts[2].z = 0.0f;

    tri->mesh.faces[0].v[0] = 0;
    tri->mesh.faces[0].v[1] = 1;
    tri->mesh.faces[0].v[2] = 2;

    tri->mesh.InvalidateGeomCache();
    tri->mesh.InvalidateTopologyCache();

    // Set the valididty to just this instant because the mesh generation
    // might work next frame.
    // wv.tri->SetChannelValidity(ALL_CHANNELS, valid);
    tri->SetChannelValidity(TOPO_CHAN_NUM, valid);
    tri->SetChannelValidity(GEOM_CHAN_NUM, valid);
    tri->SetChannelValidity(TEXMAP_CHAN_NUM, valid);
    tri->SetChannelValidity(MTL_CHAN_NUM, valid);
    tri->SetChannelValidity(SELECT_CHAN_NUM, valid);
    tri->SetChannelValidity(SUBSEL_TYPE_CHAN_NUM, valid);
    tri->SetChannelValidity(DISP_ATTRIB_CHAN_NUM, valid);
    tri->SetChannelValidity(VERT_COLOR_CHAN_NUM, valid);

    os->obj = tri;

    // os->obj->UpdateValidity(OBJ_CHANNELS, valid);

    LOGIT;
    os->obj->UnlockObject();
    LOGIT;
  }

  LOGITM("Leaving ModifyObject()");
}

// ----------------------------------------------------------------------------

// Return TRUE is this particle should be skipped for trail generation
//
BOOL GhostTrails::isSkippedTrail(int nTrail, GTWorkingValues& wv) {
  int startPart = wv.startParticle - 1;

  if (nTrail < startPart) return TRUE;

  if (((nTrail - startPart) % wv.skipParticle) != 0) return TRUE;

  if (wv.trailSource->isTrailStationary(nTrail, wv.minTrailTime, wv.t,
                                        GetTicksPerFrame(), wv.caLevelTimes))
    return TRUE;

  return FALSE;
}

// ----------------------------------------------------------------------------

void GhostTrails::initializeWorkingValues(GTWorkingValues& wv) {
  //
  // Initialize the GTWorkingValues object in preparation
  // for the trail generation calculations
  //
  LOGITM("Entering initializeWorkingValues()");

  LOGIT;
  pblock->GetValue(pb_generate_map, wv.t, wv.texturing, FOREVER);
  pblock->GetValue(pb_generate_age_map, wv.t, wv.ageTexturing, FOREVER);
  pblock->GetValue(pb_use_pflow_mtlids, wv.t, wv.usePFlowMtlIDs, FOREVER);

  LOGIT;
  // 0. Get Map Type
  wv.mapType = eString;
  int mt;
  pblock->GetValue(pb_maptype, wv.t, mt, FOREVER);
  switch (mt) {
    case 0:
      wv.mapType = eString;
      break;
    case 1:
      wv.mapType = eStretch;
      break;
  }

  LOGIT;
  // 1. Get Type Moving or Still?
  wv.bIsPathAnchored = FALSE;
  pblock->GetValue(pb_type, wv.t, wv.bIsPathAnchored, FOREVER);

  LOGIT;
  // 3. Get FlipNormal
  wv.flipNormals = FALSE;
  pblock->GetValue(pb_flipnormals, wv.t, wv.flipNormals, FOREVER);

  LOGIT;

  // 4. Get nSplineSegments
  wv.nSplineSegments = 6;
  BOOL bUseRenderSteps = FALSE;
  pblock->GetValue(GhostTrails::pb_splinesteps_render_on, 0, bUseRenderSteps,
                   FOREVER);
  if (bUseRenderSteps && inRenderMode)
    pblock->GetValue(pb_splinesteps_render, wv.t, wv.nSplineSegments, FOREVER);
  else
    pblock->GetValue(pb_splinesteps, wv.t, wv.nSplineSegments, FOREVER);

  LOGIT;
  // 5. Get fURepeat
  wv.fURepeat = 1.0;
  pblock->GetValue(pb_urepeat, wv.t, wv.fURepeat, FOREVER);

  LOGIT;
  // 5.5. Get iNumMtlIDs
  wv.iNumMtlIDs = 1;
  pblock->GetValue(pb_nummtls, wv.t, wv.iNumMtlIDs, FOREVER);

  LOGIT;
  // 6. Get fVRepeat
  wv.fVRepeat = 1.0;
  // pblock->GetValue(pb_vrepeat, wv.t, fVRepeat, FOREVER);

  LOGIT;
  // 7. Get easyfade - hardcoded at the moment.
  wv.bEasyFade = TRUE;

  LOGIT;
  // 8. Get skip & start values
  wv.startParticle = 1;
  pblock->GetValue(pb_startpart, wv.t, wv.startParticle, FOREVER);
  wv.skipParticle = 1;
  pblock->GetValue(pb_skippart, wv.t, wv.skipParticle, FOREVER);

  wv.nMotionlessLevels = 0;

  LOGIT;
  // 10. Get the level times.
  wv.caLevelTimes.clear();

  wv.fOverlapPercentage = 0.0;
  wv.fLevelsIfShowAll = 0.0;
  wv.ticksStartTime = 0;
  wv.ticksEndTime = 0;

  LOGIT;
  // Construct new tri-object.
  wv.tri = CreateNewTriObject();
  if (!wv.tri) throw CModifyException();

  LOGIT;
  if (!GetShapeAtTime(wv.theNode, wv.pShape, wv.t, wv.nSplineSegments)) {
    LOGITM("Exception");
    throw CModifyException();
  }

  LOGIT;
  wv.polys = wv.pShape.numLines;
  wv.levelVerts = wv.levelFaces = wv.levelTVerts = 0;
  wv.verts = wv.faces = wv.tVerts = 0;

  // Set up the trail source. This is either a spline or particle
  // source.
  if (IsParticleTrails()) {
    LOGIT;
    wv.trailSource = new ParticleTrailSource(&partState);
  } else {
    LOGIT;
    wv.trailSource = new SplineTrailSource(wv.theNode);
  }
  LOGITM("Leaving initializeWorkingValues()");
}

// ----------------------------------------------------------------------------

void GhostTrails::buildMeshFaces(GTWorkingValues& wv) {
  //
  // Create all required faces for the GT mesh at a point
  // in time
  //

  if (!wv.tri) return;

  Mesh& mesh = wv.tri->GetMesh();

  // Create the faces!
  int face = 0;
  int baseVert = 0;
  int TVFaceCount = 0;
  int ageTVFaceCount = 0;

  // count the number of active trails so far
  int activeTrailCount = 0;

  for (int nTrail = 0; nTrail < wv.trailSource->numTrails(); nTrail++) {
    // skip stationary trails.
    if (isSkippedTrail(nTrail, wv)) continue;

    // Check if we're using PFlow material IDs or not
    // TODO: Add param for this.
    MtlID mtlID = 0;
    if (wv.usePFlowMtlIDs) {
      mtlID = wv.trailSource->getTrailMtlID(nTrail);
    } else {
      mtlID = nTrail % wv.iNumMtlIDs;
    }

    for (int poly = 0; poly < wv.polys; ++poly) {
      PolyLine& line = wv.pShape.lines[poly];
      if (!line.numPts) continue;
      int pieces = line.Segments();
      int closed = line.IsClosed();
      int segVerts = pieces + ((closed) ? 0 : 1);
      int segTVerts = pieces + 1;

      for (int level = 0; level < (float)wv.caLevelTimes.size() - 1;
           ++level) {
        int sm = 0;  // Initial smoothing group
        BOOL firstSmooth = (line.pts[0].flags & POLYPT_SMOOTH) ? TRUE : FALSE;

        for (int piece = 0; piece < pieces; ++piece) {
          int v1 = baseVert + piece;
          int v2 = baseVert + ((piece + 1) % segVerts);
          int v3 = v1 + segVerts;
          int v4 = v2 + segVerts;

          // If the vertex is not smooth, go to the next group!
          BOOL thisSmooth = line.pts[piece].flags & POLYPT_SMOOTH;
          // MtlID mtl = useShapeIDs ? line.pts[piece].GetMatID() : 2;
          if (piece > 0 && !thisSmooth) {
            sm++;
            if (sm > 2) sm = 1;
          }
          DWORD smoothGroup = 1 << sm;
          // Advance to the next smoothing group right away
          if (sm == 0) sm++;
          // Special case for smoothing from first segment
          if (piece == 1 && thisSmooth) smoothGroup |= 1;
          // Special case for smoothing from last segment
          if ((piece == pieces - 1) && firstSmooth) smoothGroup |= 1;

          mesh.faces[face].setSmGroup(1);
          mesh.faces[face].setMatID(mtlID);
          if (!wv.flipNormals) {
            mesh.faces[face].setEdgeVisFlags(1, 1, 0);
            mesh.faces[face++].setVerts(v1, v2, v4);
          } else {
            mesh.faces[face].setEdgeVisFlags(0, 1, 1);
            mesh.faces[face++].setVerts(v1, v4, v2);
          }

          mesh.faces[face].setSmGroup(1);
          mesh.faces[face].setMatID(mtlID);
          if (!wv.flipNormals) {
            mesh.faces[face].setEdgeVisFlags(0, 1, 1);
            mesh.faces[face++].setVerts(v1, v4, v3);
          } else {
            mesh.faces[face].setEdgeVisFlags(1, 1, 0);
            mesh.faces[face++].setVerts(v1, v3, v4);
          }

          // DebugPrint("BV:%d V:%d v1:%d v2:%d v3:%d v4:%d\n",baseVert, vert,
          // v1, v2, v3, v4);
          if (wv.texturing) {
            // Where in the tvert array this trail's tverts start
            int baseTVert =
                activeTrailCount * wv.caLevelTimes.size() * wv.levelTVerts;

            // int tv1 = baseTVert + piece;
            int tv1 = baseTVert + ((pieces + 1) * level) + piece;
            int tv2 = tv1 + 1;
            int tv3 = tv1 + segTVerts;
            int tv4 = tv2 + segTVerts;

            if (tv4 >= mesh.numTVerts) {
              DebugPrint(_T("Bzzzt! - gone over the UV verts\n"));
              tv1 = tv2 = tv3 = tv4 = 0;
            }

            if (!wv.flipNormals) {
              mesh.tvFace[TVFaceCount++].setTVerts(tv1, tv2, tv4);
              mesh.tvFace[TVFaceCount++].setTVerts(tv1, tv4, tv3);
            } else {
              mesh.tvFace[TVFaceCount++].setTVerts(tv1, tv4, tv2);
              mesh.tvFace[TVFaceCount++].setTVerts(tv1, tv3, tv4);
            }
          }

          if (wv.ageTexturing) {
            UVVert* ageUVs = mesh.mapVerts(AGE_CHAN);

            // where in the age tvert array this trail's tverts start
            int baseTVertAge =
                activeTrailCount * wv.caLevelTimes.size() * wv.levelTVerts;

            int atv1 = baseTVertAge + ((pieces + 1) * level) + piece;
            int atv2 = atv1 + 1;
            int atv3 = atv1 + pieces + 1;
            int atv4 = atv3 + 1;

            /*
            DebugPrint("ATV == %d(%.2f, %.2f), %d(%.2f, %.2f), %d(%.2f, %.2f),
            %d(%.2f, %.2f)\n", atv1, ageUVs[atv1].x, ageUVs[atv1].y, atv2,
            ageUVs[atv2].x, ageUVs[atv2].y, atv3, ageUVs[atv3].x,
            ageUVs[atv3].y, atv4, ageUVs[atv4].x, ageUVs[atv4].y );
            */

            if (atv4 >= mesh.getNumMapVerts(AGE_CHAN)) {
              DebugPrint(_T("Bzzzt! - gone over the AGE verts\n"));
              atv1 = atv2 = atv3 = atv4 = 0;
            }

            TVFace* ageFaces = mesh.mapFaces(AGE_CHAN);

            if (ageFaces) {
              if (!wv.flipNormals) {
                ageFaces[ageTVFaceCount++].setTVerts(atv1, atv2, atv4);
                ageFaces[ageTVFaceCount++].setTVerts(atv1, atv4, atv3);
              } else {
                ageFaces[ageTVFaceCount++].setTVerts(atv1, atv4, atv2);
                ageFaces[ageTVFaceCount++].setTVerts(atv1, atv3, atv4);
              }
            }
          }
        }

        baseVert += segVerts;
        // baseTVert += segTVerts;
      }
      baseVert += segVerts;  // Increment to next poly start (skips last verts
                             // of this poly)
                             // baseTVert += segTVerts;
    }

    activeTrailCount++;
  }
  if (face != wv.faces) throw CModifyException();
}

// ----------------------------------------------------------------------------

void GhostTrails::calculateTrailTimes(GTWorkingValues& wv) {
  //
  // Fill in wv.caLevelTimes with the times (in ticks)
  // of each trail level.
  //

  if (!wv.bIsPathAnchored) {
    // Calculate times for normal trails.

    BOOL bRes = gGetStartAndEndTimes(pblock, GetCOREInterface()->GetAnimRange(),
                                     &wv.ticksStartTime, &wv.ticksEndTime, NULL,
                                     NULL, inRenderMode);

    // No idea why the time had to be within the active range, commenting it out
    // as we can be outside the range when rendering super-sampling for image
    // blur.
    // if((bRes == FALSE) || (wv.t < wv.ticksStartTime) || (wv.t >
    // wv.ticksEndTime))
    if (bRes == FALSE) throw CModifyException();

    float numFrames = 0.0f;
    float fLevelsPerFrame = 1.0;

    // a) get levelsPerFrame
    BOOL bUseRenderSegs = FALSE;
    pblock->GetValue(GhostTrails::pb_segsperframe_render_on, 0, bUseRenderSegs,
                     FOREVER);
    if (bUseRenderSegs && inRenderMode)
      pblock->GetValue(pb_segsperframe_render, 0, fLevelsPerFrame, FOREVER);
    else
      pblock->GetValue(pb_segsperframe, 0, fLevelsPerFrame, FOREVER);

// b) get numFrames  - it *could* be animated
#ifdef MAX2012
    if (pblock->GetControllerByID(pb_frames))  // does it have a controller
    {
      BOOL retVal = pblock->GetValue(pb_frames, wv.t, numFrames, FOREVER);
      pblock->GetControllerByID(pb_frames)->GetValue(wv.t, &numFrames, FOREVER);
    }
#else
    if (pblock->GetController(pb_frames))  // does it have a controller
    {
      BOOL retVal = pblock->GetValue(pb_frames, wv.t, numFrames, FOREVER);
      pblock->GetController(pb_frames)->GetValue(wv.t, &numFrames, FOREVER);
    }
#endif
    else {
      int foo;
      pblock->GetValue(pb_frames, wv.t, foo, FOREVER);

      numFrames = static_cast<float>(foo);
    }

    if ((numFrames <= 0.0f)) {
      // bit of a hack if there's no frames, force there to be
      // two levels at the frame time so we get a sensible mesh.
      // 3dsmax2008 gets the validity wrong if we pass it an
      // empty mesh and never regenerates it again.
      wv.caLevelTimes.push_back(wv.t);
      wv.caLevelTimes.push_back(wv.t);
      wv.minTrailTime = wv.t;
      wv.fLevelsIfShowAll = 2;
      wv.fOverlapPercentage = 0.0f;
    } else {
      // Count how many levels?
      float fLevels = numFrames * fLevelsPerFrame;
      for (int level = 0; (float)level <= fLevels; ++level) {
        // Get the time for this level.
        TimeValue levelTime = static_cast<TimeValue>(
            wv.t - (level * 1.0f / fLevelsPerFrame * GetTicksPerFrame()));
        wv.minTrailTime = levelTime;
        wv.caLevelTimes.push_back(levelTime);
      }

      // always add an overlap.
      wv.minTrailTime =
          wv.t - static_cast<TimeValue>(numFrames * GetTicksPerFrame());
      if (wv.caLevelTimes.size() > 0) {
        if (wv.minTrailTime != wv.caLevelTimes[wv.caLevelTimes.size() - 1])
          wv.caLevelTimes.push_back(wv.minTrailTime);
      }

      wv.fLevelsIfShowAll = fLevels;
      wv.fOverlapPercentage = fLevels - (int)fLevels;
    }

  } else {
    // Build time list for anchored trails.

    // Carefully calculate the variables from param block
    double fLevels = 1.0;
    double fTicksPerLevel = 1.0;

    BOOL bRes1 = gGetStartAndEndTimes(
        pblock, GetCOREInterface()->GetAnimRange(), &wv.ticksStartTime, NULL,
        &wv.fLevelsIfShowAll, NULL, inRenderMode);
    Interval curTime(wv.ticksStartTime, wv.t);
    BOOL bRes2 = gGetStartAndEndTimes(pblock, curTime, &wv.ticksStartTime,
                                      &wv.ticksEndTime, &fLevels,
                                      &fTicksPerLevel, inRenderMode);

    if (bRes1 == FALSE || bRes2 == FALSE) throw CModifyException();

    for (int level = 0; level <= fLevels; ++level) {
      TimeValue levelTime =
          wv.ticksStartTime + static_cast<TimeValue>(level * fTicksPerLevel);
      wv.caLevelTimes.push_back(levelTime);
    }

    // always add an overlap.
    wv.caLevelTimes.push_back(wv.ticksEndTime);
    wv.minTrailTime = wv.ticksStartTime;

    wv.fOverlapPercentage = fLevels - (int)fLevels;
  }

  if (wv.caLevelTimes.size() < 2) throw CModifyException();

  int siz1 = wv.caLevelTimes.size();
  int siz2 = this->caElimVStretchCache.size();

  if (IsParticleTrails())
    wv.bUsingCache = FALSE;
  else {
    // Keep the mapping constant in world space as the
    // anchored trail grows.
    ElimVStretch(wv);

    int levelCount = wv.caLevelTimes.size();
    int cacheCount = this->caElimVStretchCache.size();

    wv.bUsingCache =
        ((levelCount <= cacheCount) && wv.bIsPathAnchored) ? TRUE : FALSE;

    // if(!wv.bUsingCache)
    //{
    //	DebugPrint("No Cache!\n");
    //}
  }

  // Debug print the level times
  /*
  DebugPrint("[=====\n");
  for(int i = 0; i < wv.caLevelTimes.size(); i++)
  {
          DebugPrint("levelTime[%2d] = %02.2f\n", i,
  (float)((float)wv.caLevelTimes[i] / (float)GetTicksPerFrame()));
  }
  DebugPrint("=====]\n");
  */
}

// ----------------------------------------------------------------------------

void GhostTrails::buildMeshUVAgeVertices(GTWorkingValues& wv) {
  //
  // Create vertexes for a mapping channel that maps according to
  // trail age. We map U from 0-1 across the spline and V from
  // 0-1 along the "age" of the spline so that V = 0 for a segment that
  // has just been "born" and V==1 for a segment that is just about to
  // die.
  //

  if (!wv.ageTexturing) return;

  Mesh& mesh = wv.tri->GetMesh();

  int numTimes = wv.caLevelTimes.size();

  int numAgeVerts = wv.numActiveTrails * numTimes * wv.levelTVerts;

  mesh.setMapSupport(AGE_CHAN, TRUE);
  mesh.setNumMapVerts(AGE_CHAN, numAgeVerts, FALSE);

  int vertCount = 0;

  for (int trailIdx = 0; trailIdx < wv.trailSource->numTrails();
       trailIdx++)  // for each trail...
  {
    // skip stationary trails.
    if (isSkippedTrail(trailIdx, wv)) continue;

    for (int vv = 0; vv < numTimes;
         vv++)  // for each segment along the trail...
    {
      for (int uu = 0; uu < wv.levelTVerts; uu++)  // for each spline step...
      {
        UVVert uv;

        if (wv.levelTVerts > 1) {
          uv.x = wv.fURepeat * (float)uu / (float)(wv.levelTVerts - 1);
        } else
          uv.x = 0.0f;

        if (numTimes > 1) {
          TimeValue currentTime = wv.caLevelTimes[vv];
          TimeValue trailAgeAtTime =
              wv.trailSource->getTrailAgeAtTime(trailIdx, currentTime);
          TimeValue trailLifetime = wv.trailSource->getTrailLifeTime(trailIdx);

          if (trailAgeAtTime < 0) {
            // we're outside the lifetime of the particle - after it died in
            // other words
            uv.y = 1.0f;
          } else if (trailLifetime != 0) {
            uv.y = (float)trailAgeAtTime / (float)trailLifetime;
          } else
            uv.y = 0.0f;

          // It's 0.99 instead of 1.0 to try and prevent wrap-around errors.
          uv.y = std::clamp(uv.y, 0.0f, 0.99f);

          /*// work out how far along the length this level is (in time)
          TimeValue startTime	= wv.caLevelTimes[0];
          TimeValue endTime	= wv.caLevelTimes[numTimes - 1];

          float vValue = 0.0f;
          if(endTime == startTime)
                  vValue = 0.0f;
          else
                  vValue = fabs((float)(thisTime - startTime) / (float)(endTime
          - startTime));

          uv.y = vValue;
          */
        } else
          uv.y = 0.0f;

        uv.z = 0.0f;

        mesh.setMapVert(AGE_CHAN, vertCount, uv);

        // DebugPrint("AGE VERT %d = (%.2f, %.2f)\n", vertCount, uv.x, uv.y);

        vertCount++;
      }
    }
  }
}

// ----------------------------------------------------------------------------

void GhostTrails::buildMeshUVVertices(GTWorkingValues& wv) {
  //
  // Create all required UV verts for the GT mesh at a point
  // in time
  //

  if (!wv.texturing) return;

  int tvertex = 0;

  if (!wv.tri) throw CModifyException();

  Mesh& mesh = wv.tri->GetMesh();

  for (int trailIdx = 0; trailIdx < wv.trailSource->numTrails();
       trailIdx++)  // for each trail...
  {
    // skip stationary trails.
    if (isSkippedTrail(trailIdx, wv)) continue;

    // Handle stretch mapping.
    bool bIsStretchMapping = false;
    int nStationaryLevels = 0;
    if (wv.mapType == GhostTrails::eStretch) {
      bIsStretchMapping = true;
      nStationaryLevels =
          wv.trailSource->getNumStationaryLevels(trailIdx, wv.caLevelTimes);
    }

    for (int poly = 0; poly < wv.polys; ++poly) {
      PolyLine& line = wv.pShape.lines[poly];
      if (!line.numPts) continue;

      // a. Create tex verts
      int tp;
      int texPts = line.numPts + (line.IsClosed() ? 1 : 0);
      float* texPt = new float[texPts];
      float lastPt = (float)(texPts - 1);
      float cumLen = 0.0f;
      float totLen = line.CurveLength();
      Point3 prevPt = line.pts[0].p;

      // Generate the the U's
      for (tp = 0; tp < texPts; ++tp) {
        int ltp = tp % line.numPts;
        if (tp == (texPts - 1))
          texPt[tp] = 1.0f * wv.fURepeat;
        else {
          Point3& pt = line.pts[ltp].p;
          cumLen += Length(pt - prevPt);
          texPt[tp] = (cumLen * wv.fURepeat) / totLen;
          prevPt = pt;
        }
      }

      // ATLASSERT( wv.caLevelTimes.size()>=2 );

      // DebugPrint("\n About to dump %f levels\n", wv.caLevelTimes.size() +
      // wv.fOverlapPercentage); DebugPrint("Dumping Level Times, in frames and
      // ticks\n"); DebugPrint("Trail type: %s Trail...\n", wv.bIsPathAnchored?
      // "Anchored":"Moving");

      // Generate the the V's
      float flevels = (float)wv.caLevelTimes.size();
      for (int level = 0; level < wv.caLevelTimes.size(); ++level) {
        double tV;
        if (wv.bEasyFade) {
          int d = (int)wv.caLevelTimes[level];
          double f = (float)d / GetTicksPerFrame();

          if ((level == wv.caLevelTimes.size() - 1) && wv.bUsingCache) {
            // In the final time in our series, we are given a time that
            // has no precalculated cache assigned to it - we are in between two
            // entries.  we can't linear interpolate, cos the time
            // parameterization may not be even  so, for this special case we
            // FULLY CALCULATE IT.
            TimeValue distanceStep =
                abs((wv.caLevelTimes[level - 1] - wv.caLevelTimes[level]) / 5);
            double fDistanceBeyondLevel = GetDistanceOverRange(
                wv.theNode, wv.caLevelTimes[level - 1], wv.t, distanceStep);
            double fDistanceToNextLevel =
                GetDistanceOverRange(wv.theNode, wv.caLevelTimes[level - 1],
                                     wv.caLevelTimes[level], distanceStep);

            if (MaxUtils::fpEquals(fDistanceToNextLevel, 0.0))
              tV = caElimVStretchCache[level - 1];
            else
              tV = caElimVStretchCache[level - 1] +
                   fDistanceBeyondLevel / fDistanceToNextLevel *
                       (caElimVStretchCache[level] -
                        caElimVStretchCache[level - 1]);
          } else {
            if (wv.bUsingCache)  // anchored trails.
            {
              tV = caElimVStretchCache[level];
            } else {
              if (bIsStretchMapping) {
                // TODO
                if (level >=
                    (wv.caLevelTimes.size() - 1 - nStationaryLevels))
                  tV = 1.0f;
                else
                  tV = ((float)level) /
                       (wv.fLevelsIfShowAll - nStationaryLevels);

              } else {
                if (level == (wv.caLevelTimes.size() - 1)) {
                  tV = (level - wv.fOverlapPercentage) / wv.fLevelsIfShowAll;
                } else {
                  tV = ((float)level) / wv.fLevelsIfShowAll;
                }
              }
            }
          }
        } else {
          // OLD CODE FOR THE one texture repeat per segment system.
          tV = (float)level * wv.fVRepeat;
          if (level == wv.caLevelTimes.size() - 1) {
            tV = (float)(level - 1 + wv.fOverlapPercentage) * wv.fVRepeat;
          }

          tV = tV / (float)(level - 1 + wv.fOverlapPercentage);
        }

        for (tp = 0; tp < texPts; ++tp) {
          float theU, theV;

          theU = texPt[tp];               // clamp(texPt[tp], 0.0f, 1.0f);
          theV = static_cast<float>(tV);  // clamp(tV, 0.0f, 1.0f);
          mesh.setTVert(tvertex++, UVVert(theU, theV, 0.0f));
        }
      }
      delete[] texPt;
    }
  }
}

// ----------------------------------------------------------------------------

void GhostTrails::buildMeshVertices(GTWorkingValues& wv) {
  //
  // Create all required vertices for the GT mesh at a point
  // in time
  //

  if (!wv.tri) return;

  int vert = 0;
  Mesh& mesh = wv.tri->GetMesh();

  // Get the NodeTM of the shape at time T.
  Matrix3 level0TM = wv.theNode->GetObjectTM(wv.t);

  // Create the vertices!
  for (int nTrail = 0; nTrail < wv.trailSource->numTrails(); nTrail++) {
    // skip stationary trails.
    if (isSkippedTrail(nTrail, wv)) continue;

    for (int poly = 0; poly < wv.polys; ++poly) {
      PolyLine& line = wv.pShape.lines[poly];
      if (!line.numPts) continue;

      int lverts = line.numPts;

      // Get the basic dimension stuff
      float amount = 100.0f;
      float zSize = (float)fabs(amount);
      float baseZ = 0.0f;
      if (amount < 0.0f) baseZ = amount;

      // b) Create geom verts
      for (int level = 0; level < wv.caLevelTimes.size(); ++level) {
        // Get the node TM for this level.
        TimeValue levelTime = wv.caLevelTimes[level];
        Matrix3 levelLTM = wv.trailSource->getTrailTM(
            nTrail, levelTime,
            wv.caLevelTimes);  // theNode->GetObjectTM(levelTime);

        // Rotate the particle splines of non-particle flow particle systems
        // to follow the path of the particle. We have to do this because the
        // maxscript interface to 3dsmax4-style particle systems only returns
        // positions to us.
        if (IsParticleTrails() &&
            (partState.getSourceSystemType() !=
             ParticleSystemState::PS_PARTICLEFLOW) &&
            (wv.caLevelTimes.size() > 0)) {
          AffineParts aa;
          decomp_affine(levelLTM, &aa);
          Point3 keyPos = aa.t;

          // TODO: Don't calculate tangents on adjacent keys if
          // they are in the same position - try and find the next
          // (or previous) key in a different place.

          // The tangent along the path of this particle at time t
          // (this is used to rotate the spline to follow the path)
          Point3 keyTangent(1.0f, 0.0f, 0.0f);

          if (level == 0) {
            // First key
            Point3 nextKeyPos = getNextDifferentKeyPos(
                wv.trailSource, nTrail, wv.caLevelTimes, level, 1);

            keyTangent = nextKeyPos - keyPos;
          } else if (level == (wv.caLevelTimes.size() - 1)) {
            // last key
            Point3 previousKeyPos = getNextDifferentKeyPos(
                wv.trailSource, nTrail, wv.caLevelTimes, level, -1);

            keyTangent = keyPos - previousKeyPos;
          } else {
            Point3 nextKeyPos = getNextDifferentKeyPos(
                wv.trailSource, nTrail, wv.caLevelTimes, level, 1);
            Point3 previousKeyPos = getNextDifferentKeyPos(
                wv.trailSource, nTrail, wv.caLevelTimes, level, -1);

            keyTangent = nextKeyPos - previousKeyPos;
          }

          levelLTM.SetRow(0, Normalize(keyTangent));
          levelLTM.SetRow(1, Normalize(Point3(0, 0, 1) ^ levelLTM.GetRow(0)));
          levelLTM.SetRow(2, levelLTM.GetRow(0) ^ levelLTM.GetRow(1));

          Point3 tmp = levelLTM.GetRow(0);
          levelLTM.SetRow(0, -levelLTM.GetRow(2));
          levelLTM.SetRow(2, tmp);

          levelLTM.SetRow(3, keyPos);
        }

        PolyShape pShapet;

        if (!GetShapeAtTime(wv.theNode, pShapet, levelTime, wv.nSplineSegments))
          break;

        PolyLine& linet = pShapet.lines[poly];

        Point3 offset =
            Point3(0.0f, 0.0f,
                   baseZ + (float)level / (float)wv.caLevelTimes.size() -
                       1 * zSize);

        for (int v = 0; v < lverts; ++v) {
          Point3 p0 = line.pts[v].p * level0TM;
          Point3 pL = linet.pts[v].p;

          offset = pL - p0;
          offset = -offset;

          line.pts[v].aux =
              vert;  // Gives the capper this vert's location in the mesh!
          mesh.setVert(vert++, linet.pts[v].p * levelLTM * Inverse(level0TM));
        }
      }
    }
  }
  assert(vert == wv.verts);

  PolyShape dummyShape;
  GetShapeAtTime(wv.theNode, dummyShape, wv.t, wv.nSplineSegments);
}

// ----------------------------------------------------------------------------

//
// Get the position of the next key that is in a different position to the key
// at currentIdx.
//
// step == 1   : get the next key
// step == -1  : get the previous key
//
Point3 GhostTrails::getNextDifferentKeyPos(
    ITrailSource* pTrailSource, int nTrail,
    std::vector<TimeValue>& levelTimes, int currentIdx, int step) {
  if (!pTrailSource) {
    return Point3(0.0f, 0.0f, 0.0f);
  }

  Point3 curPoint;
  AffineParts cAA;

  Matrix3 curTM = pTrailSource->getTrailTM(nTrail, levelTimes[currentIdx],
                                           levelTimes);
  decomp_affine(curTM, &cAA);

  curPoint = cAA.t;

  Point3 workingPoint = curPoint;
  int workingIdx = currentIdx + step;

  while ((workingIdx >= 0) && (workingIdx < levelTimes.size())) {
    Matrix3 workingTM = pTrailSource->getTrailTM(
        nTrail, levelTimes[workingIdx], levelTimes);
    AffineParts wAA;
    decomp_affine(workingTM, &wAA);
    workingPoint = wAA.t;

    if (Length(workingPoint - curPoint) > 0.01) break;

    workingIdx += step;
  }

  return workingPoint;
}

// ----------------------------------------------------------------------------

void GhostTrails::BeginEditParams(IObjParam* ip, ULONG flags,
                                  Animatable* prev) {
  // LOGIT;
  this->ip = ip;
  GhostTrailsDesc.BeginEditParams(ip, this, flags, prev);

  // init the array - the UserDlgProcs will set the values.
  for (int i = 0; i < NUM_ROLLUPS; i++) rollupHandles[i] = 0;

  ghosttrails_param_blk.SetUserDlgProc(
      ghosttrails_map_main, new GhostTrailsDlgProc(this, ghosttrails_map_main));
  ghosttrails_param_blk.SetUserDlgProc(
      ghosttrails_map_meshparams,
      new GhostTrailsDlgProc(this, ghosttrails_map_meshparams));
  ghosttrails_param_blk.SetUserDlgProc(
      ghosttrails_map_apply,
      new GhostTrailsDlgProc(this, ghosttrails_map_apply));
  ghosttrails_param_blk.SetUserDlgProc(
      ghosttrails_particle, new GhostTrailsDlgProc(this, ghosttrails_particle));
  // LOGIT;
}

// ----------------------------------------------------------------------------

void GhostTrails::EndEditParams(IObjParam* ip, ULONG flags, Animatable* next) {
  LOGIT;
  GhostTrailsDesc.EndEditParams(ip, this, flags, next);
  this->ip = NULL;
  LOGIT;
}

// ----------------------------------------------------------------------------

// From ReferenceMaker
#ifdef MAX2015
RefResult GhostTrails::NotifyRefChanged(const Interval& changeInt,
                                        RefTargetHandle hTarget, PartID& partID,
                                        RefMessage message, BOOL propagate)
#else
RefResult GhostTrails::NotifyRefChanged(Interval changeInt,
                                        RefTargetHandle hTarget, PartID& partID,
                                        RefMessage message)
#endif
{
  LOGIT;

  char dbgstr[256];

  if (GhostTrails::isDebugLogging()) {
    if (hTarget) {
      MSTR s;
      hTarget->GetClassName(s);
      char* converted = MaxUtils::wcharToChar(s.data());
      sprintf(dbgstr, "hTarget is a %s, msg = %09X, part = %llu", converted,
              message, partID);
      delete converted;
      LOGITM(dbgstr);
    } else {
      sprintf(dbgstr, "hTarget is NULL. msg = %09X, part = %llu", message,
              partID);
      LOGITM(dbgstr);
    }
  }

  switch (message) {
    case REFMSG_CHANGE:

      LOGIT;
      // see if this message came from a changing parameter in the pblock,
      if (hTarget == pblock) {
        LOGIT;
        ParamID changing_param = pblock->LastNotifyParamID();
        LOGIT;

        if (changing_param == GhostTrails::pb_particle_node) {
          // We don't need to propagate changes when the particle system
          // reference changes as it doesn't actually change the state of the
          // object.
          LOGITM("REF_STOP");
          return REF_STOP;
        } else {
          LOGIT;
          return REF_SUCCEED;
        }
      }

      break;

    case REFMSG_OBJECT_CACHE_DUMPED:
      // Dependents don't need to know about cache changes as the external
      // representation of the object hasn't changed. Adding this prevents
      // weird crashes during rendering when the referenced particle system
      // sends this message over and over.
      LOGITM("REFMSG_OBJECT_CACHE_DUMPED");
      return REF_STOP;

    case REFMSG_WANT_SHOWPARAMLEVEL:
      LOGITM("REFMSG_WANT_SHOWPARAMLEVEL");
      return REF_STOP;
  }
  LOGIT;
  return (REF_SUCCEED);
}

// ----------------------------------------------------------------------------

// From Object
BOOL GhostTrails::HasUVW() {
  // TODO: Return whether the object has UVW coordinates or not
  return TRUE;
}

// ----------------------------------------------------------------------------

void GhostTrails::SetGenUVW(BOOL sw) {
  if (sw == HasUVW()) return;
  // TODO: Set the plugin internal value to sw
}

// ----------------------------------------------------------------------------

IOResult GhostTrails::Load(ILoad* iload) {
  // LOGIT;
  // TODO: Add code to allow plugin to load its data

  partState.load(iload);
  return IO_OK;
}

// ----------------------------------------------------------------------------

IOResult GhostTrails::Save(ISave* isave) {
  LOGIT;
  partState.save(isave);
  return IO_OK;
}

// ----------------------------------------------------------------------------

#ifdef MAX2013
#ifdef MAX2022
const TCHAR* GhostTrails::GetObjectName(bool isLocalized) const
#else
const MCHAR* GhostTrails::GetObjectName()
#endif
#else
TCHAR* GhostTrails::GetObjectName()
#endif
{
  return GetString(IDS_CLASS_NAME);
}

// ----------------------------------------------------------------------------

ChannelMask GhostTrails::ChannelsUsed() { return PART_GEOM | PART_TOPO; }

// ----------------------------------------------------------------------------

ChannelMask GhostTrails::ChannelsChanged() { return PART_ALL; }

// ----------------------------------------------------------------------------

Class_ID GhostTrails::InputType() {
  // Only operate on shapes for now.
  return genericShapeClassID;
}

// ----------------------------------------------------------------------------

BOOL GhostTrails::ChangeTopology() { return TRUE; }

// ----------------------------------------------------------------------------

CreateMouseCallBack* GhostTrails::GetCreateMouseCallBack() { return NULL; }

// ----------------------------------------------------------------------------

Class_ID GhostTrails::ClassID() { return GHOSTTRAILS_CLASS_ID; }

// ----------------------------------------------------------------------------

SClass_ID GhostTrails::SuperClassID() { return OSM_CLASS_ID; }

// ----------------------------------------------------------------------------

#ifdef MAX2022
void GhostTrails::GetClassName(MSTR& s, bool localized) const { s = localized ? GetString(IDS_CLASS_NAME) : _T("GhostTrails"); }

#else
void GhostTrails::GetClassName(TSTR& s) { s = GetString(IDS_CLASS_NAME); }
#endif

// ----------------------------------------------------------------------------

int GhostTrails::NumSubs() { return 1; }

// ----------------------------------------------------------------------------

#ifdef MAX2022
TSTR GhostTrails::SubAnimName(int i, BOOL isLocalized) { return GetString(IDS_PARAMS1); }
#else
TSTR GhostTrails::SubAnimName(int i) { return GetString(IDS_PARAMS1); }
#endif

// ----------------------------------------------------------------------------

Animatable* GhostTrails::SubAnim(int i) {
  if (GhostTrails::isDebugLogging()) {
    char buf[256];
    sprintf(buf, "Requesting SubAnim %d", i);
    LOGITM(buf);
  }
  return pblock;
}

// ----------------------------------------------------------------------------

int GhostTrails::NumRefs() { return 1; }

// ----------------------------------------------------------------------------

RefTargetHandle GhostTrails::GetReference(int i) { return pblock; }

// ----------------------------------------------------------------------------

void GhostTrails::SetReference(int i, RefTargetHandle rtarg) {
  pblock = (IParamBlock2*)rtarg;
}

// ----------------------------------------------------------------------------

int GhostTrails::NumParamBlocks() {
  // return number of ParamBlocks in this instance
  return 1;
}

// ----------------------------------------------------------------------------

IParamBlock2* GhostTrails::GetParamBlock(int i) {
  // return i'th ParamBlock
  return pblock;
}

// ----------------------------------------------------------------------------

IParamBlock2* GhostTrails::GetParamBlockByID(BlockID id) {
  return (pblock->ID() == id) ? pblock : NULL;
}

// ----------------------------------------------------------------------------

void GhostTrails::DeleteThis() {
  LOGIT;
  delete this;
  LOGIT;
}

// ----------------------------------------------------------------------------

INode* GhostTrails::GetNode() {
  // Try and find the node that this instance of the GhostTrails modifier is
  // applied to.

  LOGIT;
  TSTR nodename;

  INode* answer;

  NotifyDependents(FOREVER, (PartID)&nodename, REFMSG_GET_NODE_NAME);

  if (!ip)
    answer = GetCOREInterface()->GetINodeByName(nodename);
  else
    answer = ip->GetINodeByName(nodename);

  if (answer == 0) return 0;

  // this just confirms it genuinely has the modifier

  // Get object from node. Abort if no object.
  Object* ObjectPtr = answer->GetObjectRef();
  Modifier* foundMod = 0;

  if (!ObjectPtr) return NULL;

  // Is derived object ?
  if (ObjectPtr->SuperClassID() == GEN_DERIVOB_CLASS_ID) {
    // Yes -> Cast.
    IDerivedObject* DerivedObjectPtr = static_cast<IDerivedObject*>(ObjectPtr);

    // Iterate over all entries of the modifier stack.
    int ModStackIndex = 0;
    while (ModStackIndex < DerivedObjectPtr->NumModifiers()) {
      // Get current modifier.
      Modifier* ModifierPtr = DerivedObjectPtr->GetModifier(ModStackIndex);

      // Is this GhostTrails ?
      if (ModifierPtr->ClassID() == ClassID()) {
        // Yes -> Exit.
        foundMod = ModifierPtr;
        break;
      }

      // Next modifier stack entry.
      ModStackIndex++;
    }
  }

  LOGIT;
  if (foundMod == this)
    return answer;
  else
    return 0;
}

// ----------------------------------------------------------------------------

BOOL GhostTrails::GetShapeAtTime(INode* theNode, PolyShape& theShape,
                                 TimeValue tim, int splineSteps) {
  LOGITM("Entering GetShapeAtTime()");
  // evaluate the spline shape just below the GT modifier

  if (!theNode) return FALSE;

  // LOGIT;
  Object* pObj = theNode->GetObjectRef();

  // LOGIT;
  if (!pObj) {
    LOGITM("Exiting GetShapeAtTime() - FALSE (pObj NULL)");
    return FALSE;
  }

  if (pObj->SuperClassID() != GEN_DERIVOB_CLASS_ID) {
    LOGITM("Exiting GetShapeAtTime() - FALSE");
    return FALSE;
  }

  IDerivedObject* pDerObj = (IDerivedObject*)pObj;

  IDerivedObject* pFoundDerObj = 0;  // the derived obj with the GT modifier
  int GTModIdx = -1;                 // index of the GT modifier

  // LOGIT;
  while (pDerObj && !pFoundDerObj) {
    for (int i = 0; i < pDerObj->NumModifiers(); i++) {
      Modifier* pMod = pDerObj->GetModifier(i);
      if (pMod == this) {
        GTModIdx = i;
        pFoundDerObj = pDerObj;
        // break;
      }
      // DebugPrint("[%s]: Modifier %d is %s\n", (const
      // char*)theNode->GetName(), i, (const char*)pMod->GetName());
    }
    if (!pFoundDerObj) {
      // if we haven't found it, go down to the base obj and see if it's
      // a derivedObj
      pDerObj = (IDerivedObject*)pDerObj->GetObjRef();
      if (pDerObj && pDerObj->SuperClassID() != GEN_DERIVOB_CLASS_ID)
        pDerObj = 0;
    }
  }

  // LOGIT;
  if (!pFoundDerObj || (GTModIdx == -1))
    return FALSE;  // couldn't find modifier.

  // We want to eval the object at one place in the stack below the GT modifier
  // (this is so we can pick up any modifiers on the spline (edit spline etc.)).
  ObjectState os;

  int numMods = pFoundDerObj->NumModifiers();

  // LOGIT;
  if (numMods <= 1) {
    // Case #1: GT is the only modifier, then use the objref. The objref
    // might be an IDerivedObject again but we don't care because if it's
    // the one below the GT modifier we know it resolves to a spline.
    // LOGIT;
    os = pFoundDerObj->GetObjRef()->Eval(tim);
  } else {
    // Case #2: There is more than one mod on this object so we want to
    // eval from the one below the GT
    if ((GTModIdx + 1) < numMods) {
      // LOGIT;
      os = pFoundDerObj->Eval(tim, GTModIdx + 1);
    } else {
      // LOGIT;
      os = pFoundDerObj->GetObjRef()->Eval(tim);
    }
  }

  ShapeObject* shape = (ShapeObject*)os.obj;

  // LOGIT;
  PolyShape pShape;
  shape->MakePolyShape(tim, pShape, splineSteps);
  ShapeHierarchy hier;
  pShape.OrganizeCurves(tim, &hier);

  // Need to flip the reversed curves in the shape!
  // LOGIT;
  pShape.Reverse(hier.reverse);

  theShape = pShape;

  // LOGIT;
  theNode->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
  // LOGIT;
  theNode->InvalidateWS();

  LOGITM("Leaving GetShapeAtTime()");
  return TRUE;
}

// ----------------------------------------------------------------------------

BOOL GhostTrailsDlgProc::Apply(HWND hWnd) {
  BOOL bTex;
  ssm->pblock->GetValue(GhostTrails::pb_apply_tex_b, TimeValue(0), bTex,
                        FOREVER);

  float fVRepeat = 1.0;
  ssm->pblock->GetValue(GhostTrails::pb_apply_tex_vrepeat, TimeValue(0),
                        fVRepeat, FOREVER);

  BOOL bFade;
  ssm->pblock->GetValue(GhostTrails::pb_apply_fade_b, TimeValue(0), bFade,
                        FOREVER);

  int iFadeStyle = 1;
  // ssm->pblock->GetValue(pb_apply_fade_inout, TimeValue(0), iFadeStyle,
  // FOREVER);

  float fFadeMidpoint = 1.0;
  ssm->pblock->GetValue(GhostTrails::pb_apply_fade_mp, TimeValue(0),
                        fFadeMidpoint, FOREVER);

  BOOL bTextureSet = FALSE;

  if (bTex) {  // fade only valid if tex is checked

    // Create New Std Material
    Mtl* pMtl = ssm->GetNode()->GetMtl();

    if (!pMtl) {
      pMtl = NewDefaultStdMat();
      pMtl->SetName(_T("GhostTrails"));
    } else if (pMtl && pMtl->GetSubTexmap(ID_OP) &&
               _tcsstr(pMtl->GetName(), _T("GhostTrails"))) {
      int retval =
          MessageBox(hWnd, _T("Do you wish to Overwrite the opacity map of "),
                     _T("Overwrite"), MB_YESNOCANCEL);
      switch (retval) {
        case IDCANCEL:
          return FALSE;
        case IDNO:
          pMtl = NULL;
      };
    }

    if (pMtl) {
      // pDefBmpTex->BitmapLoadDlg();
      TCHAR buffer[MAX_PATH] = {0};

      //_tcscpy(buffer, ssm->ip->GetDir(APP_MATLIB_DIR) );
      // TCHAR* pValue = NULL;
      // ssm->pblock->GetValue(pb_apply_tex_edit, TimeValue(0), pValue,
      // FOREVER); _tcscat(buffer, pValue);

      // Get the value from the windowText, incase hasn't been enacted.
      GetWindowText(GetDlgItem(hWnd, IDC_APPLY_TEX_EDIT), buffer, MAX_PATH);

      // Create New Bitmap Tex
      BitmapTex* pDefBmpTex = NewDefaultBitmapTex();

      // Set it up
      {
        if (!pDefBmpTex) {
          int retval =
              MessageBox(hWnd, _T("Failed to create texture of bitmap"),
                         _T("Failed"), MB_OK);
          return FALSE;
        }
        StdUVGen* pUVGen = NULL;
        pUVGen = pDefBmpTex->GetUVGen();
        if (pUVGen) {
          pUVGen->SetFlag(U_WRAP, true);
          pUVGen->SetUScl(1.0, TimeValue(0));

          pUVGen->SetFlag(V_WRAP, true);
          pUVGen->SetVScl(fVRepeat, TimeValue(0));
        }

        // Set and validate
        pDefBmpTex->SetMapName(buffer);
#ifdef MAX2013
        const MCHAR* testName = pDefBmpTex->GetMapName();
#else
        TCHAR* testName = pDefBmpTex->GetMapName();
#endif
        if (0) {  //_tcscmp(buffer, testName)!=0){
          int retval = MessageBox(hWnd, _T("Failed to set path of texture"),
                                  _T("Failed"), MB_OK);
          return FALSE;
        }

        pDefBmpTex->ReloadBitmapAndUpdate();
        Bitmap* test = pDefBmpTex->GetBitmap(TimeValue(0));
      }

      if (IsTex(pDefBmpTex)) {
        // Set the texmap in the material as DIFFUSE
        pMtl->SetSubTexmap(ID_DI, (Texmap*)pDefBmpTex);
        pMtl->SetActiveTexmap((Texmap*)pDefBmpTex);
        bTextureSet = TRUE;

        // Set the Material - it is likely to be already set.
        ssm->GetNode()->SetMtl(pMtl);
        ssm->ip->ActivateTexture((Texmap*)pDefBmpTex, pMtl, -1);
      }

      // Set our node to that of our new material
      ssm->ip->RedrawViews(ssm->ip->GetTime());
    }
  }

  // If fade
  if (bFade) {
    Mtl* pMtl = ssm->GetNode()->GetMtl();

    if (!pMtl) {
      pMtl = NewDefaultStdMat();
      pMtl->SetName(_T("GhostTrails"));
    } else if (pMtl && pMtl->GetSubTexmap(ID_OP) &&
               _tcsstr(pMtl->GetName(), _T("GhostTrails"))) {
      int retval =
          MessageBox(hWnd, _T("Do you wish to Overwrite the opacity map of "),
                     _T("Overwrite"), MB_YESNOCANCEL);
      switch (retval) {
        case IDCANCEL:
          return FALSE;
        case IDNO:
          pMtl = NULL;
      };
    }

    if (pMtl) {
      // Create New Gradient Tex
      GradTex* pGradTex = NewDefaultGradTex();

      // Set it up
      pGradTex->SetMidPoint(fFadeMidpoint, TimeValue(0));
      if (iFadeStyle) {
        pGradTex->SetNumSubTexmaps(3);

        pGradTex->SetColor(0, Color(1, 1, 1), TimeValue(1));
        pGradTex->SetColor(1, Color(0, 0, 0), TimeValue(1));
        pGradTex->SetColor(2, Color(0, 0, 0), TimeValue(1));

      } else {
        pGradTex->SetNumSubTexmaps(3);
        pGradTex->SetColor(0, Color(0, 0, 0), TimeValue(1));
        pGradTex->SetColor(1, Color(0, 0, 0), TimeValue(1));
        pGradTex->SetColor(2, Color(1, 1, 1), TimeValue(1));
      }

      // Set the texmap in the material as OPACITY
      if (IsTex(pGradTex)) {
        // Set the texmap in the material as OPACITY
        pMtl->SetSubTexmap(ID_OP, (Texmap*)pGradTex);
        pMtl->SetActiveTexmap((Texmap*)pGradTex);
        bTextureSet = TRUE;

        // Set the Material - it is likely to be already set.
        ssm->GetNode()->SetMtl(pMtl);
        if (!bTextureSet) pMtl->SetActiveTexmap((Texmap*)pGradTex);
      }
    }
  }
  ssm->ip->RedrawViews(ssm->ip->GetTime());

  if (bTextureSet) {
    if (bTextureSet == 1) {
      MessageBox(hWnd, _T("Effects have been successfully applied"),
                 _T("Success"), MB_OK);
    } else {
      MessageBox(hWnd, _T("Effects have been successfully applied"),
                 _T("Success"), MB_OK);
    }
  }

  return bTextureSet;
}

// ----------------------------------------------------------------------------

int GhostTrails::ElimVStretch(GTWorkingValues& wv) {
  if (!wv.theNode) return 1;

  // Return error if not in Anchored Mode
  BOOL bIsAnchored = FALSE;
  pblock->GetValue(pb_type, TimeValue(0), bIsAnchored, FOREVER);
  if (!bIsAnchored) return 1;

  Interval animRange;
  if (ip)
    animRange = ip->GetAnimRange();
  else
    animRange = GetCOREInterface()->GetAnimRange();

  // Carefully calculate the variables from param block
  TimeValue ticksStartFrame = 0;
  TimeValue ticksEndFrame = 0;
  double fTicksPerLevel = 1.0;
  double fLevels = 1.0;
  BOOL bRes =
      gGetStartAndEndTimes(pblock, animRange, &ticksStartFrame, &ticksEndFrame,
                           &fLevels, &fTicksPerLevel, inRenderMode);

  if (bRes == FALSE) {
    return 2;
  }

  // granularity of time interval used for distance along path
  // calculations.
  TimeValue distanceStep = static_cast<TimeValue>(fTicksPerLevel / 2.0f);
  if (distanceStep < GetTicksPerFrame()) {
    // sanity check (so the calcs don't take forever).
    distanceStep = GetTicksPerFrame();
  }

  // Get total distance
  double fTotalDistance = GetDistanceOverRange(wv.theNode, ticksStartFrame,
                                               ticksEndFrame, distanceStep);

  // Reset the cache
  caElimVStretchCache.clear();

  double accumulatedDistance = 0.0;
  if (!MaxUtils::fpEquals(fLevels, 0.0)) {
    for (int level = 0; level < wv.caLevelTimes.size(); ++level) {
      TimeValue levelTime = wv.caLevelTimes[level];
      TimeValue lastLevelTime;

      if (level == 0)
        lastLevelTime = levelTime;
      else
        lastLevelTime = wv.caLevelTimes[level - 1];

      double percentage = static_cast<double>(level) /
                          static_cast<double>(wv.caLevelTimes.size());

      if (!MaxUtils::fpEquals(fTotalDistance, 0.0)) {
        double fDistanceAtLevel = GetDistanceOverRange(
            wv.theNode, lastLevelTime, levelTime, distanceStep);
        accumulatedDistance += fDistanceAtLevel;
        percentage = accumulatedDistance / fTotalDistance;
      }
      caElimVStretchCache.push_back(percentage);
    }
  }

  // always add 1.0 to cap it - it doesn't matter if it never gets used.
  caElimVStretchCache.push_back(1.0);

  // I don't understand why this redraw is here - AKR 24-10-2005
  //
  // Update the viewport, so the user can see.
  // ip->RedrawViews(ip->GetTime());

  return 0;
}

// ----------------------------------------------------------------------------

//
// Return the (approximate) length of the path travelled by the node over
// the time range (start,end).
//
// Calculate by summing linear segments of duration "step".
//
double GhostTrails::GetDistanceOverRange(INode* pNode, TimeValue start,
                                         TimeValue end, TimeValue step) {
  // INode* pNode = GetNode();

  // sanity checks
  if (end <= start) {
    return 0.0;
  }
  if (step <= 0) {
    step = std::max(GetTicksPerFrame(), end - start);
  }
  if (step > (end - start)) {
    step = (end - start);
  }

  Point3 oldPosition = getPosAtTime(start, pNode);

  // int iSamplingIncrement = GetTicksPerFrame() / 2;
  double fTotalDistance = 0.0;

  TimeValue i = start;
  while (i <= end) {
    Point3 newPosition = getPosAtTime(i, pNode);
    Point3 diff = newPosition - oldPosition;
    fTotalDistance += diff.Length();
    oldPosition = newPosition;

    if (i >= end) break;

    // put last point exactly at the end.
    if ((i + step) > end) step = end - i;

    i += step;
  }

  // for(TimeValue i = start; i <= end; i += step)
  //{
  //	Point3 newPosition = getPosAtTime(i, pNode);
  //	Point3 diff = newPosition - oldPosition;
  //	fTotalDistance += diff.Length();
  //	oldPosition = newPosition;
  //}

  return fTotalDistance;
}

// ----------------------------------------------------------------------------

Point3 GhostTrails::getPosAtTime(TimeValue time, INode* node) {
  // Return the position portion of the TM matrix of a node at
  // a certain time.
  if (!node) return Point3(0.0f, 0.0f, 0.0f);

  Matrix3 tm = node->GetNodeTM(time);

  AffineParts aa;

  decomp_affine(tm, &aa);

  return aa.t;
}

// ----------------------------------------------------------------------------

BOOL GhostTrails::IsParticleTrails() {
  // Return TRUE if the particle trails feature is
  // enabled for this modifier, FALSE otherwise
  BOOL bIsParticle = FALSE;
  if (pblock && pblock->GetValue(pb_particle_trails_b, TimeValue(0),
                                 bIsParticle, FOREVER))
    return bIsParticle;
  return FALSE;
}

// ----------------------------------------------------------------------------

void GhostTrails::RecalculateParticleTrails(HWND hWndButton) {
  ScopeLock scopeLock(&critSec);

  isRebuildingTrails = TRUE;
  partState.clearParticles();

  INode* pParticleNode = NULL;
  if (pblock)
    pblock->GetValue(pb_particle_node, TimeValue(0), pParticleNode, FOREVER);
  if (!pParticleNode) {
    // no particle node to operate on
    isRebuildingTrails = FALSE;
    return;
  }

  Interval animRange = GetCOREInterface()->GetAnimRange();

  Tab<INode*>* pEventList = NULL;
  Tab<INode*> eventList;

  // Only pass in the event list if we're not doing all events
  BOOL useAllPFEvents;
  pblock->GetValue(pb_useallpf, 0, useAllPFEvents, FOREVER);
  if (!useAllPFEvents) {
    // build a list of event nodes
    int ct = pblock->Count(pb_pfeventlist);
    for (int i = 0; i < ct; i++) {
      INode* node = NULL;
      pblock->GetValue(pb_pfeventlist, 0, node, FOREVER, i);
      if (node) {
        eventList.Append(1, &node);
      }
    }
    pEventList = &eventList;
  }

  NullProgress nullProgress;
  ButtonProgress buttonProgress(hWndButton);
  ParticleStateBuilder::buildState(partState, pParticleNode, animRange.Start(),
                                   animRange.End(), GetTicksPerFrame(),
                                   &buttonProgress, pEventList);

  // DlgProgress dlgProgress(&partState, animRange, pParticleNode, pEventList,
  // limited);  dlgProgress.doRecalc();

  isRebuildingTrails = FALSE;

  NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
  // if(theNode)
  //	theNode->InvalidateWS();
}

// ----------------------------------------------------------------------------

void GhostTrails::SnapToSpline() {
  if (partState.numParticles() > 0)
    partState.makeTestSpline(_T("GTSpline"));
  else {
    MessageBox(GetCOREInterface()->GetMAXHWnd(), _T("No particle animation."),
               _T("Ghost Trails"), MB_OK);
  }
}

// ----------------------------------------------------------------------------

void GhostTrails::NotifyInputChanged(Interval changeInt, PartID partID,
                                     RefMessage message, ModContext* mc) {}

// ----------------------------------------------------------------------------

static FPInterfaceDesc gt_mixininterface(GT_FO_INTERFACE, _T("foo"), 0,
                                         &GhostTrailsDesc, FP_MIXIN,
                                         gt_regen_particle_trails,
                                         _T("regenerate_particle_trails"), 0,
                                         TYPE_VOID, 0, 0, p_end);

// ----------------------------------------------------------------------------

FPInterfaceDesc* GTInterface::GetDesc() { return &gt_mixininterface; }

// ----------------------------------------------------------------------------

int GhostTrails::RenderBegin(TimeValue t, ULONG flags) {
  LOGIT;
  inRenderMode = TRUE;
  Animatable::RenderBegin(t, flags);

  flowEventStash.SetCount(0);

  // build a list of event nodes
  int ct = pblock->Count(pb_pfeventlist);
  while (pblock->Count(pb_pfeventlist) > 0) {
    INode* node = NULL;
    pblock->GetValue(pb_pfeventlist, 0, node, FOREVER, 0);
    if (node) flowEventStash.Append(1, &node, 1);
    pblock->Delete(pb_pfeventlist, 0, 1);
    // if (node)
    //{
    //	eventList.Append(1, &node);
    //}
  }

  SetAFlag(A_RENDER);
  ghosttrails_param_blk.InvalidateUI();
  NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
  return 1;
}

// ----------------------------------------------------------------------------

int GhostTrails::RenderEnd(TimeValue t) {
  LOGIT;
  ClearAFlag(A_RENDER);
  inRenderMode = FALSE;
  Animatable::RenderEnd(t);

  for (int i = flowEventStash.Count() - 1; i >= 0; i--) {
    pblock->Append(pb_pfeventlist, 1, &(flowEventStash[i]));
  }

  ghosttrails_param_blk.InvalidateUI();
  NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
  return 1;
}

// ----------------------------------------------------------------------------

BOOL GhostTrails::isDebugLogging() { return debugLogging; }

// ----------------------------------------------------------------------------

void GhostTrails::setDebugLogging(BOOL isLogging) { debugLogging = isLogging; }

// ----------------------------------------------------------------------------
