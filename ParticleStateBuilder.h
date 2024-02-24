#ifndef _PARTICLESTATEBUILDER_H_
#define _PARTICLESTATEBUILDER_H_

class IProgress;

class MAXNode;

// ---------------------------------------------------------------------------
//
// ParticleStateBuilder.h
//
// class ParticleStateBuilder does the work of building a ParticleSystemState
// from a 3dsmax Particle System object
//
// ---------------------------------------------------------------------------

class ParticleStateBuilder {
 public:
  enum { PSB_ERROR = 0, PSB_OK = 1 };

  // Fill in a ParticleSystemState from a particle system.
  //
  // state		- the ParticleSystemState to fill in
  // pPSystem		- the Particle System node to use
  // startTime	- the time to start sampling particle animation
  // endTime		- the time to end sampling
  // stepTime		- the sampling rate (in ticks)
  // pProgress	- pointer to a progress notifier
  // pfEventList	- list of PFlow events to build a state for (can be
  // NULL)
  static int buildState(ParticleSystemState& pstate, INode* pParticleSystem,
                        TimeValue startTime, TimeValue endTime,
                        TimeValue stepTime, IProgress* pProgress,
                        Tab<INode*>* pfEventList = NULL);

  static bool isParticleFlowNode(INode* node);

 protected:
  // called by buildState()
  static int buildParticleFlowState(ParticleSystemState& pstate,
                                    INode* pParticleSystem, TimeValue startTime,
                                    TimeValue endTime, TimeValue stepTime,
                                    IProgress* pProgress,
                                    Tab<INode*>* pfEventList);

  // Find the material ID from a PFlow particle group node and return it. IF
  // it can't be found, return 1.
  static int findParticleMtlID(TimeValue curtime, INode* particleGroupNode,
                               int groupIdx);

  // called by buildState()
  static int buildParticleSystemState(ParticleSystemState& pstate,
                                      INode* pParticleSystem,
                                      TimeValue startTime, TimeValue endTime,
                                      TimeValue stepTime, IProgress* pProgress);

 private:
  // "special" value for particle age to indicate
  // there is no particle alive at the moment.
  static const int AGE_NO_PARTICLE;

  // Return the age of the idx'th particle at the current time.
  static TimeValue particleAge(MAXNode*, int idx);

  // Return the position of the idx'th particle at the current time
  static Point3 particlePosition(MAXNode*, int idx);

  // Helper function to add a particle state to the ParticleSystemState
  static void commitParticle(ParticleSystemState& state,
                             ParticleState& partsArray);

  // set the maxscript time context.
  static void setMaxScriptTime(TimeValue);

  ParticleStateBuilder();
  ParticleStateBuilder(const ParticleStateBuilder&);
};

// ---------------------------------------------------------------------------

#endif  // _PARTICLESTATEBUILDER_H_
