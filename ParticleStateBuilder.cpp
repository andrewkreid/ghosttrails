#include "Max.h"

#include <map>
#include <vector>

#ifdef MAX2012
#include "maxscript/foundation/numbers.h"
#include "maxscript/maxscript.h"
#include "maxscript/maxwrapper/mxsobjects.h"
//#include "maxscript/Parser.h"
#else
#include "maxscrpt/MAXObj.h"
#include "maxscrpt/Numbers.h"
#include "maxscrpt/Parser.h"
#include "maxscrpt/maxscrpt.h"
#endif

#ifdef MAX6
#include "IParticleObjectExt.h"
#include "ParticleFlow/IPFSystem.h"
#include "ParticleFlow/IParticleChannelMaterialIndex.h"
#include "ParticleFlow/IParticleGroup.h"
#endif

#include "IProgress.h"
#include "Logging.h"
#include "ParticleState.h"
#include "ParticleStateBuilder.h"

// ---------------------------------------------------------------------------

// Used internally for error conditions.
class ParticleStateBuilderException {};

// ---------------------------------------------------------------------------
//
// class ParticleStateBuilder does the work of building a ParticleSystemState
// from a 3dsmax Particle System object
//
// ---------------------------------------------------------------------------

const int ParticleStateBuilder::AGE_NO_PARTICLE = INT_MAX;

// ---------------------------------------------------------------------------

bool ParticleStateBuilder::isParticleFlowNode(INode* node) {
  LOGIT;
#ifdef MAX6
  // Are we a Particle Flow object?
  IParticleObjectExt* pIParticle =
      (IParticleObjectExt*)GetParticleObjectExtInterface(node->GetObjectRef());
  if (pIParticle) {
    DebugPrint(_T("Got IParticleObjectExt - age(0) = %d\n"),
               pIParticle->GetParticleAgeByBornIndex(0));
    return true;
  } else
    return false;
#else
  return false;
#endif
}

// ---------------------------------------------------------------------------
// Fill in a ParticleSystemState from a particle system.
//
// state		- the ParticleSystemState to fill in
// pPSystem		- the Particle System node to use
// startTime	- the time to start sampling particle animation
// endTime		- the time to end sampling
// stepTime		- the sampling rate (in ticks)
// pProgress	- the progress notifier (can be NULL)
// version?
//
int ParticleStateBuilder::buildState(ParticleSystemState& state,
                                     INode* pParticleSystem,
                                     TimeValue startTime, TimeValue endTime,
                                     TimeValue stepTime, IProgress* pProgress,
                                     Tab<INode*>* pfEventList) {
  // Are we a Particle Flow object?
  LOGIT;
  if (isParticleFlowNode(pParticleSystem)) {
    LOGIT;
    return buildParticleFlowState(state, pParticleSystem, startTime, endTime,
                                  stepTime, pProgress, pfEventList);
  } else {
    LOGIT;
    return buildParticleSystemState(state, pParticleSystem, startTime, endTime,
                                    stepTime, pProgress);
  }
}

// ---------------------------------------------------------------------------
// Fill in a ParticleSystemState from a Particle Flow object
//
// state		- the ParticleSystemState to fill in
// pPSystem		- the Particle System node to use
// startTime	- the time to start sampling particle animation
// endTime		- the time to end sampling
// stepTime		- the sampling rate (in ticks)
// pProgress	- the progress notifier (can be NULL)
//
int ParticleStateBuilder::buildParticleFlowState(
    ParticleSystemState& state, INode* pParticleSystem, TimeValue startTime,
    TimeValue endTime, TimeValue stepTime, IProgress* pProgress,
    Tab<INode*>* pfEventList) {
#ifdef MAX6
  //
  // Some Notes
  //
  // UpdateParticles() looks like the equivalent of "at time"
  //
  // NumParticlesGenerated() for the "real" particle count??
  // GetParticleBornIndex(int i) gets the unique ID of a particle at index i
  // GetParticleTMByBornIndex(int id) may give us bank,spin etc for free!
  //
  // Easier than 3dsmax4 particles (if it all works as advertised)
  //

  char buf[256];

  // Map used to keep track of a particle's index in its group. This is needed
  // for getting the material IDs
  std::map<INode*, int> groupParticleCount;

  BOOL bRenderStateForced = FALSE;

  LOGIT;
  try {
    TimeValue curTime =
        GetCOREInterface()->GetTime();  // so we can set it back afterwards.

    state.setSourceSystemType(ParticleSystemState::PS_PARTICLEFLOW);

    int retval = PSB_OK;

    LOGIT;
    state.clearParticles();

    LOGIT;
    IParticleObjectExt* pIParticle =
        (IParticleObjectExt*)GetParticleObjectExtInterface(
            pParticleSystem->GetObjectRef());

    // We have to put the particle system into the renders state so that
    // PFOperatorMaterialFrequency::Proceed will set the materialID channel
    IPFSystem* iSystem = GetPFSystemInterface(pParticleSystem->GetObjectRef());
    if (iSystem && !iSystem->IsRenderState()) {
      iSystem->SetRenderState(true);
      bRenderStateForced = true;
    }

    if (!pIParticle) return PSB_ERROR;

    LOGIT;
    if (pProgress) pProgress->progressStart();

    LOGIT;

    // Get the total number of particles. This should be the number generated by
    // the end of the time period
    LOGIT;
    pIParticle->UpdateParticles(pParticleSystem, endTime);
    LOGIT;
    int numParticlesBorn = pIParticle->NumParticlesGenerated();

    DebugPrint(_T("NumParticlesGenerated() = %d\n"), numParticlesBorn);

    LOGIT;
    state.setNumParticles(numParticlesBorn);

    LOGIT;
    // Loop through the time segment and add keys
    for (int t = startTime; t <= endTime; t += stepTime) {
      if (pProgress) {
        if (pProgress->isCancelled()) {
          pProgress->progressEnd();
          state.clearParticles();
          return PSB_ERROR;
        }
        pProgress->progressNotify(t - startTime, endTime - startTime);
      }

      sprintf(buf, "Time = %06d", t);
      LOGITM(buf);

      pIParticle->UpdateParticles(pParticleSystem, t);
      LOGIT;

      for (int bornID = 0; bornID < pIParticle->NumParticlesGenerated();
           bornID++) {
        // Get the matching BornID for this particle.
        // int bornID = pIParticle->GetParticleBornIndex(part);

        if (bornID >= 0) {
          if (bornID > (state.numParticles() - 1)) {
            DebugPrint(_T("RESIZING to %d\n"), bornID + 1);
            state.setNumParticles(bornID + 1);
          }

          ParticleState& partState = state.getParticle(bornID);

          sprintf(buf, "BornID = %06d", bornID);
          LOGITM(buf);

          int partIdx = -1;
          if (pIParticle->HasParticleBornIndex(bornID, partIdx)) {
            INode* curGroup = pIParticle->GetParticleGroup(partIdx);
            int idxInGroup = pIParticle->GetParticleIndex(bornID);

            // Save the particle's material ID if we can find it.
            partState.setMtlID(findParticleMtlID(t, curGroup, idxInGroup));

            // Check that the particle is in a valid group (if we're filtering
            // by group)
            if (pfEventList && curGroup) {
              BOOL useParticle = FALSE;
              for (int k = 0; k < pfEventList->Count(); k++) {
                sprintf(buf, "EventList = %06d", k);
                LOGITM(buf);
                if (curGroup == pfEventList->operator[](k)) {
                  LOGIT;
                  useParticle = TRUE;
                  break;
                }
                LOGIT;
              }
              if (useParticle != TRUE) {
                LOGIT;
                continue;
              }
            }
          }

          LOGIT;
          TimeValue age = pIParticle->GetParticleAgeByBornIndex(bornID);
          LOGIT;

          /*
          if(bornID == 0)
          {
                  DebugPrint(" AGE %d at %d = %d\n", bornID, t /
          GetTicksPerFrame(), age);
          }
          */

          // NB: Annoyingly, the age is 0 both at birth and after death.

          if ((age > 0) ||
              ((age == 0) && (partState.numKeys() == 0)))  // <-- particle born
          {
            LOGIT;
            Matrix3* tm = pIParticle->GetParticleTMByBornIndex(bornID);
            LOGIT;
            if (tm) {
              LOGIT;
              partState.addKey(ParticlePositionKey(t, *tm, age));
            } else {
              LOGIT;
              // particle is alive, so let's save a key for it.
              Point3* pos = pIParticle->GetParticlePositionByBornIndex(bornID);
              LOGIT;
              if (pos) {
                LOGIT;
                partState.addKey(ParticlePositionKey(t, *pos, age));
                // DebugPrint("\tKEY\t(%.2f,%.2f,%.2f) for %d at %d\n",
                // pos->x,pos->y,pos->z, bornID, t / GetTicksPerFrame());
              }
              LOGIT;
            }
          }
        }
      }
    }

    if (bRenderStateForced && iSystem) {
      iSystem->SetRenderState(false);
    }

    LOGIT;
    pIParticle->UpdateParticles(pParticleSystem,
                                curTime);  // set back to "current" time.
    LOGIT;
  } catch (...) {
    LOGIT;
    DebugPrint(
        _T("Exception thrown in ")
        _T("ParticleStateBuilder::buildParticleFlowState\n"));
    state.clearParticles();
    return PSB_ERROR;
  }

  if (pProgress) {
    LOGIT;
    pProgress->progressEnd();
  }
  LOGIT;

  state.calculateMaximumParticleAge();
  LOGIT;

#endif  // ifdef MAX6

  return PSB_OK;
}

int ParticleStateBuilder::findParticleMtlID(TimeValue curTime,
                                            INode* particleGroupNode,
                                            int groupIdx) {
  if (!particleGroupNode) {
    return 0;
  }
  Object* particleGroupObj =
      (particleGroupNode != NULL)
          ? particleGroupNode->EvalWorldState(curTime).obj
          : NULL;

  if (!particleGroupObj) {
    return 0;
  }

  IParticleGroup* particleGroup = GetParticleGroupInterface(particleGroupObj);
  ::IObject* groupContainer = particleGroup->GetParticleContainer();

  IParticleChannelIntR* chMtlIndex =
      GetParticleChannelMtlIndexRInterface(groupContainer);
  if (chMtlIndex) {
    return chMtlIndex->GetValue(groupIdx);
  }
  return 0;
}

// ---------------------------------------------------------------------------
// Fill in a ParticleSystemState from a Particle System object
// that supports the standard Maxscript particle interface.
//
// state		- the ParticleSystemState to fill in
// pPSystem		- the Particle System node to use
// startTime	- the time to start sampling particle animation
// endTime		- the time to end sampling
// stepTime		- the sampling rate (in ticks)
// pProgress	- the progress notifier (can be NULL)
//
int ParticleStateBuilder::buildParticleSystemState(
    ParticleSystemState& state, INode* pParticleSystem, TimeValue startTime,
    TimeValue endTime, TimeValue stepTime, IProgress* pProgress) {
  LOGIT;
  int retval = PSB_OK;

  state.clearParticles();

  LOGIT;
  state.setSourceSystemType(ParticleSystemState::PS_STANDARD);

  LOGIT;
  if (!pParticleSystem) return PSB_ERROR;

  init_thread_locals();
  push_alloc_frame();
  three_typed_value_locals(
      Value * numParts,      // number of particles,
      MAXNode * maxNode,     // maxscript node for the particle system
      Value * intArgValue);  // index argument
  LOGIT;
  if (pProgress) pProgress->progressStart();

  LOGIT;

  char buf[256];
  LOGIT;
  try {
    LOGIT;
    vl.maxNode = new MAXNode(pParticleSystem);
    LOGIT;

    if (vl.maxNode) {
      LOGIT;
      // get the number of particles in the system
      vl.intArgValue = Integer::intern(1);
      vl.numParts = vl.maxNode->particlecount_vf(NULL, 0);

      int numParticles = vl.numParts->to_int();

      // make a working array of ParticleStates
      std::vector<ParticleState> workingPartsArray;
      workingPartsArray.resize(numParticles);

      LOGIT;

      // ...and an array of particle ages so we can track when particles
      // start and finish.
      std::vector<TimeValue> particleAges;
      particleAges.resize(numParticles, AGE_NO_PARTICLE);

      TimeValue currentTime = startTime;

      LOGIT;
      while (currentTime <= endTime) {
        sprintf(buf, "currentTime = %06d, max = %06d", currentTime, endTime);
        LOGITM(buf);

        if (pProgress) {
          if (pProgress->isCancelled()) {
            pProgress->progressEnd();
            state.clearParticles();
            throw ParticleStateBuilderException();
          }
          pProgress->progressNotify(currentTime - startTime,
                                    endTime - startTime);
        }

        LOGIT;
        // Set maxscript's time context
        setMaxScriptTime(currentTime);

        for (int i = 0; i < numParticles; i++) {
          sprintf(buf, "i = %06d, numParticles = %06d", i, numParticles);
          LOGITM(buf);

          TimeValue curParticleAge = particleAge(vl.maxNode, i);

          // There are numerous cases here to do with the birth &
          // death of particles...

          if ((particleAges[i] == AGE_NO_PARTICLE) &&
              (curParticleAge == AGE_NO_PARTICLE)) {
            // This particle doesn't exist now and didn't last frame either
            // so we have nothing to do.
          } else if ((curParticleAge != AGE_NO_PARTICLE) &&
                     ((particleAges[i] == AGE_NO_PARTICLE) ||
                      ((particleAges[i] != AGE_NO_PARTICLE) &&
                       (particleAges[i] >=
                        curParticleAge))  // <-- immediate rebirth
                      )) {
            // A particle has been born (congratulations!) start saving it's
            // keys in the workingPartsArray.

            // Is there a particle in the working array for this index
            // already? (there might be - in the case where a particle dies & is
            // born again on adjacent frames).
            LOGIT;
            if (particleAges[i] != AGE_NO_PARTICLE) {
              DebugPrint(_T("Particle %d died at time %f\n"), i,
                         (float)currentTime / (float)GetTicksPerFrame());
              commitParticle(state, workingPartsArray[i]);
            }

            ParticleState& partState = workingPartsArray[i];
            Point3 partPos = particlePosition(vl.maxNode, i);

            partState.clearKeys();
            partState.setBirthTime(currentTime);
            partState.addKey(ParticlePositionKey(currentTime, partPos, 0));

            DebugPrint(_T("Particle %d born at time %.1f\n"), i,
                       (float)currentTime / (float)GetTicksPerFrame());
          } else if ((particleAges[i] != AGE_NO_PARTICLE) &&
                     (curParticleAge != AGE_NO_PARTICLE) &&
                     (curParticleAge >= particleAges[i])) {
            LOGIT;
            // adding keys to an existing particle
            ParticleState& partState = workingPartsArray[i];
            Point3 partPos = particlePosition(vl.maxNode, i);
            partState.addKey(
                ParticlePositionKey(currentTime, partPos, curParticleAge));
          } else if ((particleAges[i] != AGE_NO_PARTICLE) &&
                     (curParticleAge == AGE_NO_PARTICLE)) {
            LOGIT;
            // The particle has died (so sad!). Add it to the
            // ParticleSystemState
            DebugPrint(_T("Committed particle %d at time %f\n"), i,
                       (float)currentTime / (float)GetTicksPerFrame());
            commitParticle(state, workingPartsArray[i]);
            DebugPrint(_T("Particle %d died at time %f\n"), i,
                       (float)currentTime / (float)GetTicksPerFrame());
          } else {
            LOGIT;
            // Should never get here!
            DebugPrint(
                _T("ERROR: Invalid particle state for particle %d at time ")
                _T("%ld\n"),
                i, currentTime);
          }

          LOGIT;
          particleAges[i] = curParticleAge;
        }

        currentTime += stepTime;
      }

      LOGIT;
      // Finish up all particles still alive at the end of the sample period.
      for (int j = 0; j < numParticles; j++) {
        ParticleState& partState = workingPartsArray[j];
        if (partState.numKeys() > 0) {
          commitParticle(state, partState);
        }
      }
      LOGIT;
    }
  } catch (...) {
    LOGIT;
    DebugPrint(
        _T("Exception thrown in ")
        _T("ParticleStateBuilder::buildParticleSystemState\n"));
    state.clearParticles();
    retval = PSB_ERROR;
  }

  LOGIT;
#ifndef MAX2017
  pop_value_locals();
#endif
  LOGIT;
  pop_alloc_frame();
  LOGIT;

  if (pProgress) pProgress->progressEnd();

  LOGIT;
  state.calculateMaximumParticleAge();

  LOGIT;
  return retval;
}

// ---------------------------------------------------------------------------

// set the maxscript time context. This is equivalent to the "at time" command
//
void ParticleStateBuilder::setMaxScriptTime(TimeValue tim) {
  ((MAXScript_TLS*)TlsGetValue(thread_locals_index))->use_time_context = TRUE;
  ((MAXScript_TLS*)TlsGetValue(thread_locals_index))->current_time = tim;
}

// ---------------------------------------------------------------------------

// Return the age of the idx'th particle at the current time.
//
TimeValue ParticleStateBuilder::particleAge(MAXNode* pMaxNode, int idx) {
  LOGIT;
  assert(pMaxNode != NULL);

  TimeValue retval = AGE_NO_PARTICLE;
  Value* idxVal = Integer::intern(idx + 1);

  try {
    Value* timeVal = pMaxNode->particleage_vf(&idxVal, 1);
    if (timeVal) {
      retval = timeVal->to_timevalue();
    }
  } catch (ConversionError) {
    LOGIT;
    // probably no particle alive at this index
    // for the current time.
  } catch (...) {
    LOGIT;
    // ???
    DebugPrint(_T("UNKNOWN Exception in ParticleStateBuilder::particleAge\n"));
  }

  LOGIT;
  return retval;
}

// ---------------------------------------------------------------------------

// Return the position of a particle at the current time
//
// NB: Throws ConversionError if the i'th particle doesn't exist
//
Point3 ParticleStateBuilder::particlePosition(MAXNode* pMaxNode, int idx) {
  LOGIT;
  assert(pMaxNode != NULL);

  // Get the position of a particle and print it out.
  //
  // idx is 0 based

  Value* arg = Integer::intern(idx + 1);
  Value* retval = pMaxNode->particlepos_vf(&arg, 1);
  if (retval) {
    Point3 partLocation = retval->to_point3();
    // DebugPrint("\t\tParticle %d\t(%.2f, %.2f, %.2f)\n", idx, partLocation.x,
    // partLocation.y, partLocation.z);
    return partLocation;
  }
  LOGIT;
  return Point3(-1.0f, -1.0f, -1.0f);
}

// ---------------------------------------------------------------------------

// Add the idx'th working particle to the ParticleSystemState and reset the
// slot for (possible) reuse
//
void ParticleStateBuilder::commitParticle(ParticleSystemState& state,
                                          ParticleState& part) {
  LOGIT;
  state.addParticle(part);
  part.clearKeys();
  LOGIT;
}

// ---------------------------------------------------------------------------
