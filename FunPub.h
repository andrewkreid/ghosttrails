#include "iFnPub.h"

#define GT_FO_INTERFACE Interface_ID(0x7e1d629d, 0x1db22765)

#define GetGTInterface(obj) ((GTInterface*)obj->GetInterface(GT_FO_INTERFACE))

// function IDs
enum { gt_regen_particle_trails };

// mixin interface
class GTInterface : public FPMixinInterface {
  BEGIN_FUNCTION_MAP

  VFN_0(gt_regen_particle_trails, RegenerateParticleTrails);

  END_FUNCTION_MAP

  FPInterfaceDesc* GetDesc();

  virtual void RegenerateParticleTrails() = 0;
};
