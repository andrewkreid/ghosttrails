#ifndef _TRAILSOURCE_H
#define _TRAILSOURCE_H

#include <vector>
#include <max.h>

#include "ParticleState.h"

// ----------------------------------------------------------------------------

//
// Abstract interface for classes that provide the TM's for one or
// more trails.
//
class ITrailSource {
 public:
  virtual int numTrails() = 0;
  virtual Matrix3 getTrailTM(int trailIdx, TimeValue t,
                             std::vector<TimeValue>& leveltTimes) = 0;
  virtual TimeValue getTrailAgeAtTime(int trailIdx, TimeValue t) = 0;
  virtual TimeValue getTrailLifeTime(int trailIdx) = 0;

  virtual BOOL isTrailStationary(int trailIdx, TimeValue startT, TimeValue endT,
                                 TimeValue keyStep,
                                 std::vector<TimeValue>& levelTimes) = 0;

  virtual int getNumStationaryLevels(
      int trailIdx, std::vector<TimeValue>& levelTimes) = 0;
  virtual int getTrailMtlID(int trailIdx) = 0;
};

// ----------------------------------------------------------------------------

//
// Trail source for ordinary trails generated from the shape object's
// animation.
//
class SplineTrailSource : public ITrailSource {
 public:
  SplineTrailSource(INode*);

  virtual int numTrails();
  virtual Matrix3 getTrailTM(int trailIdx, TimeValue t,
      std::vector<TimeValue>& leveltTimes);

  virtual TimeValue getTrailAgeAtTime(int trailIdx, TimeValue t);
  virtual TimeValue getTrailLifeTime(int trailIdx);

  virtual BOOL isTrailStationary(int trailIdx, TimeValue startT, TimeValue endT,
                                 TimeValue keyStep,
      std::vector<TimeValue>& levelTimes);

  virtual int getNumStationaryLevels(int trailIdx,
      std::vector<TimeValue>& levelTimes);
  virtual int getTrailMtlID(int trailIdx);

 private:
  Point3 getTMPos(const Matrix3&);

  INode* pShapeNode;

  static float MIN_TRAIL_MOVEMENT;
};

// ----------------------------------------------------------------------------

//
// Trail source for trails generated from a particle system.
//
class ParticleTrailSource : public ITrailSource {
 public:
  ParticleTrailSource(ParticleSystemState*);

  virtual int numTrails();
  virtual Matrix3 getTrailTM(int trailIdx, TimeValue t,
      std::vector<TimeValue>& leveltTimes);

  virtual TimeValue getTrailAgeAtTime(int trailIdx, TimeValue t);
  virtual TimeValue getTrailLifeTime(int trailIdx);

  virtual BOOL isTrailStationary(int trailIdx, TimeValue startT, TimeValue endT,
                                 TimeValue keyStep,
      std::vector<TimeValue>& levelTimes);

  virtual int getNumStationaryLevels(int trailIdx,
      std::vector<TimeValue>& levelTimes);
  virtual int getTrailMtlID(int trailIdx);

 private:
  void findSurroundingKeys(ParticleState&, TimeValue, int&, int&);
  Matrix3 interpolateMatrices(const Matrix3* leftminusoneTM,
                              const Matrix3* leftTM, const Matrix3* rightTM,
                              const Matrix3* rightplusoneTM, TimeValue lTime,
                              TimeValue rTime, TimeValue cTime);

  ParticleSystemState* pPartState;
};

// ----------------------------------------------------------------------------

#endif  // _TRAILSOURCE_H
