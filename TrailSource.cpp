// ----------------------------------------------------------------------------

#include "TrailSource.h"
#include "Max.h"
#include "ParticleState.h"
#include "decomp.h"

// ----------------------------------------------------------------------------

// The minimum a trail can move before it is no longer stationary
// (for mesh culling porpoises)
//
float SplineTrailSource::MIN_TRAIL_MOVEMENT = 0.001f;

// ----------------------------------------------------------------------------

SplineTrailSource::SplineTrailSource(INode* node) { pShapeNode = node; }

// ----------------------------------------------------------------------------

int SplineTrailSource::numTrails() {
  // there is only one trail for normal spline trails, the one
  // hanging off the spline.

  return 1;
  // return 3;
}

// ----------------------------------------------------------------------------

Matrix3 SplineTrailSource::getTrailTM(
    int idx, TimeValue t, std::vector<TimeValue>& levelTimes) {
  // Return the TM of the spline at time t.
  assert(pShapeNode != NULL);

  if (levelTimes.size() <= 0) return Matrix3(1);

  // return pShapeNode->GetObjectTM(t);

  Matrix3 objTM = pShapeNode->GetObjectTM(t);

#ifdef CAM_TRAILS
  // reference time for calculating camera-space trails
  TimeValue cameraTrailRefTime = levelTimes[0];
  if (levelTimes[0] < levelTimes[levelTimes.GetSize() - 1]) {
    // moving trails and anchored trails put the leveltimes array in different
    // orders (from newest to oldest). We need to flip the ref time for
    // anchored trails.
    cameraTrailRefTime = levelTimes[levelTimes.GetSize() - 1];
  }

  INode* pCam = GetCOREInterface()->GetINodeByName("Camera01");
  if (pCam) {
    // Works for moving trails
    objTM = pShapeNode->GetObjectTM(t) * Inverse(pCam->GetNodeTM(t)) *
            pCam->GetNodeTM(cameraTrailRefTime);
  }
#endif

  return objTM;
}

// ----------------------------------------------------------------------------

BOOL SplineTrailSource::isTrailStationary(
    int trailIdx, TimeValue startT, TimeValue endT, TimeValue keyStep,
    std::vector<TimeValue>& levelTimes) {
#ifdef CAM_TRAILS
  // VV CAMTRAIL
  return FALSE;
#else

  // Return TRUE if the trail is stationary over the interval startT->endT,
  // FALSE otherwise. This is used to cull zero-length trails from mesh
  // generation. Sample the motion every keyStep ticks.

  // TODO: cache values here?

  TimeValue curT = startT;

  // get the start position
  Point3 startPos = getTMPos(pShapeNode->GetObjectTM(startT));

  while (curT <= endT) {
    curT += keyStep;

    Point3 curPos = getTMPos(pShapeNode->GetObjectTM(curT));
    if (Length(startPos - curPos) > SplineTrailSource::MIN_TRAIL_MOVEMENT) {
      // trail has moved
      return FALSE;
    }
  }

  return TRUE;
#endif
}

// ----------------------------------------------------------------------------

// Return the number of levels at the tail of the trail where the trail isn't
// moving. Used to correctly do stretch mapping.
//
int SplineTrailSource::getNumStationaryLevels(
    int trailIdx, std::vector<TimeValue>& levelTimes) {
  int count = 0;
  Matrix3 lastTM;
  Matrix3 curTM;

#ifdef CAM_TRAILS
  // VV CAMTRAIL
  return 0;
#else

  for (int level = (levelTimes.size() - 1); level > 0; --level) {
    if (level == (levelTimes.size() - 1)) {
      lastTM = getTrailTM(trailIdx, levelTimes[level], levelTimes);
    } else {
      curTM = getTrailTM(trailIdx, levelTimes[level], levelTimes);
      if (curTM.Equals(lastTM)) {
        // No change
        count++;
        lastTM = curTM;
      } else
        return count;
    }
  }

  return 0;
#endif
}

// ----------------------------------------------------------------------------

Point3 SplineTrailSource::getTMPos(const Matrix3& m) {
  // helper function to decompose a Matrix3 and return its
  // translation component.

  AffineParts parts;

  decomp_affine(m, &parts);
  return parts.t;
}

// ----------------------------------------------------------------------------

TimeValue SplineTrailSource::getTrailAgeAtTime(int, TimeValue) {
  // not meaningful for spline trails.
  return -1;
}

// ----------------------------------------------------------------------------

TimeValue SplineTrailSource::getTrailLifeTime(int) {
  // not meaningful for spline trails.
  return -1;
}

int SplineTrailSource::getTrailMtlID(int trailIdx) {
  // There is no extra data for spline trails to define Material Index.
  return 1;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

ParticleTrailSource::ParticleTrailSource(ParticleSystemState* pState) {
  pPartState = pState;
}

// ----------------------------------------------------------------------------

int ParticleTrailSource::numTrails() {
  if (!pPartState)
    return 0;
  else {
    return pPartState->numParticles();
  }
}

// ----------------------------------------------------------------------------

void ParticleTrailSource::findSurroundingKeys(ParticleState& particle,
                                              TimeValue t, int& pLeftKeyIdx,
                                              int& pRightKeyIdx) {
  // Find the two keys on either side of TimeValue t and set pLeftKeyIdx and
  // pRightKeyIdx to their indices. In the case of an exact time match, set
  // pLeftKeyIdx to the matching key and pRightKey to -1. Also do this for
  // requests outside the animation range (ie before the first or after the last
  // key).

  // NB: Assumes the keys are evenly spaced in time (and in ascending order)!!
  pLeftKeyIdx = pRightKeyIdx = -1;

  int numKeys = particle.numKeys();

  if (numKeys == 0) return;

  if (numKeys == 1) {
    pLeftKeyIdx = 0;
    return;
  }

  TimeValue startTime = particle.getKey(0).getTime();
  TimeValue endTime = particle.getKey(numKeys - 1).getTime();

  // before range?
  if (t <= startTime) {
    pLeftKeyIdx = 0;
    return;
  }

  // after range
  if (t >= endTime) {
    pLeftKeyIdx = numKeys - 1;
    return;
  }

  // get the nearest key that has time < t.
  int targetKeyIdx = (int)((float)numKeys * ((float)t - (float)startTime) /
                           ((float)endTime - (float)startTime));
  if (targetKeyIdx >= numKeys) targetKeyIdx = numKeys - 1;

  const ParticlePositionKey* foundKey = &particle.getKey(targetKeyIdx);

  // step back to a key before time t if we're not there yet.
  while ((foundKey->getTime() > t) && (targetKeyIdx >= 0)) {
    targetKeyIdx--;
    foundKey = &particle.getKey(targetKeyIdx);
  }

  // Have we got an exact match (or are we on the last key)?
  if ((foundKey->getTime() == t) || (targetKeyIdx == (numKeys - 1))) {
    pLeftKeyIdx = targetKeyIdx;
    return;
  }

  const ParticlePositionKey* nextKey = &particle.getKey(targetKeyIdx + 1);

  // sanity check
  if (!((foundKey->getTime() <= t) && (nextKey->getTime() >= t))) {
    // The two keys don't straddle the requested time
    // How can this be?
    pLeftKeyIdx = targetKeyIdx;
    return;
  }

  pLeftKeyIdx = targetKeyIdx;
  pRightKeyIdx = targetKeyIdx + 1;
}

// ----------------------------------------------------------------------------

Matrix3 ParticleTrailSource::getTrailTM(int idx, TimeValue t,
    std::vector<TimeValue>&) {
  if (!pPartState) return Matrix3(TRUE);

  int realPartIdx = idx;

  ParticleState& particle = pPartState->getParticle(realPartIdx);

  int leftKeyIdx = -1;
  int rightKeyIdx = -1;

  findSurroundingKeys(particle, t, leftKeyIdx, rightKeyIdx);

  Matrix3 tm(TRUE);

  if (leftKeyIdx == -1) {
    // Couldn't find any keys???
    return tm;
  } else if (rightKeyIdx == -1) {
    tm = particle.getKey(leftKeyIdx).getTM();
  } else {
    // To smoothly interpolate, we need the four keys surrounding the
    // desired time.
    //
    // leftminusoneTM....leftTM...t...rightTM...rightplusoneTM

    const Matrix3* leftminusoneTM = 0;
    const Matrix3* leftTM = 0;
    const Matrix3* rightTM = 0;
    const Matrix3* rightplusoneTM = 0;

    ParticlePositionKey& leftKey = particle.getKey(leftKeyIdx);
    ParticlePositionKey& rightKey = particle.getKey(rightKeyIdx);

    leftTM = &leftKey.getTM();
    rightTM = &rightKey.getTM();

    if (leftKeyIdx > 0) {
      leftminusoneTM = &(particle.getKey(leftKeyIdx - 1).getTM());
    }
    if (rightKeyIdx < (particle.numKeys() - 1)) {
      rightplusoneTM = &(particle.getKey(rightKeyIdx + 1).getTM());
    }

    tm = interpolateMatrices(leftminusoneTM, leftTM, rightTM, rightplusoneTM,
                             leftKey.getTime(), rightKey.getTime(), t);
  }

  return tm;
}

// ----------------------------------------------------------------------------

static float CubicInterpolate(float y0, float y1, float y2, float y3,
                              float mu) {
  float a0, a1, a2, a3, mu2;

  mu2 = mu * mu;
  a0 = y3 - y2 - y0 + y1;
  a1 = y0 - y1 - a0;
  a2 = y2 - y0;
  a3 = y1;

  return (a0 * mu * mu2 + a1 * mu2 + a2 * mu + a3);
}

// ----------------------------------------------------------------------------

static double CosineInterpolate(double y1, double y2, double mu) {
  double mu2;

  mu2 = (1 - cos(mu * PI)) / 2;
  return (y1 * (1 - mu2) + y2 * mu2);
}

// ----------------------------------------------------------------------------

/*
   Tension: 1 is high, 0 normal, -1 is low
   Bias: 0 is even,
         positive is towards first segment,
         negative towards the other
*/
static float HermiteInterpolate(float y0, float y1, float y2, float y3,
                                float mu, float tension = 0.0,
                                float bias = 0.0) {
  float m0, m1, mu2, mu3;
  float a0, a1, a2, a3;

  mu2 = mu * mu;
  mu3 = mu2 * mu;
  m0 = (y1 - y0) * (1 + bias) * (1 - tension) / 2;
  m0 += (y2 - y1) * (1 - bias) * (1 - tension) / 2;
  m1 = (y2 - y1) * (1 + bias) * (1 - tension) / 2;
  m1 += (y3 - y2) * (1 - bias) * (1 - tension) / 2;
  a0 = 2 * mu3 - 3 * mu2 + 1;
  a1 = mu3 - 2 * mu2 + mu;
  a2 = mu3 - mu2;
  a3 = -2 * mu3 + 3 * mu2;

  return (a0 * y1 + a1 * m0 + a2 * m1 + a3 * y2);
}

// ----------------------------------------------------------------------------

Matrix3 ParticleTrailSource::interpolateMatrices(
    const Matrix3* leftminusoneTM, const Matrix3* leftTM,
    const Matrix3* rightTM, const Matrix3* rightplusoneTM, TimeValue lTime,
    TimeValue rTime, TimeValue cTime) {
  // return the matrix gained by interpolating from (lTime,lMat)
  // to (rTime,rMat) at time cTime.
  //
  // Assumes lTime <= cTime <= rTime
  //

  if (!leftTM || !rightTM) return Matrix3(1);  // Identity

  if (cTime == lTime) return *leftTM;
  if (cTime == rTime) return *rightTM;
  if (lTime == rTime) return *leftTM;
  if ((cTime < lTime) || (cTime > rTime)) return Matrix3(1);  // Identity

  float interpFactor = fabs((float)(cTime - lTime) / (float)(rTime - lTime));

  AffineParts lminusoneParts;
  AffineParts lParts;
  AffineParts rParts;
  AffineParts rplusoneParts;

  decomp_affine(*leftTM, &lParts);
  decomp_affine(*rightTM, &rParts);

  if (leftminusoneTM)
    decomp_affine(*leftminusoneTM, &lminusoneParts);
  else {
    // If there's no key before the left key extrapolate along
    // the last known direction for the porpoises of interpolation.
    lminusoneParts.q = lParts.q;
    lminusoneParts.t = lParts.t - (rParts.t - lParts.t);
    lminusoneParts.k = lParts.k - (rParts.k - lParts.k);
  }

  if (rightplusoneTM)
    decomp_affine(*rightplusoneTM, &rplusoneParts);
  else {
    // If there's no key after the right key extrapolate along
    // the last known direction for the porpoises of interpolation.
    rplusoneParts.q = rParts.q;
    rplusoneParts.t = rParts.t + (rParts.t - lParts.t);
    rplusoneParts.k = rParts.k + (rParts.k - lParts.k);
  }

  Point3 interpPos(
      HermiteInterpolate(lminusoneParts.t.x, lParts.t.x, rParts.t.x,
                         rplusoneParts.t.x, interpFactor),
      HermiteInterpolate(lminusoneParts.t.y, lParts.t.y, rParts.t.y,
                         rplusoneParts.t.y, interpFactor),
      HermiteInterpolate(lminusoneParts.t.z, lParts.t.z, rParts.t.z,
                         rplusoneParts.t.z, interpFactor));

  Point3 interpScale(
      HermiteInterpolate(lminusoneParts.k.x, lParts.k.x, rParts.k.x,
                         rplusoneParts.k.x, interpFactor),
      HermiteInterpolate(lminusoneParts.k.y, lParts.k.y, rParts.k.y,
                         rplusoneParts.k.y, interpFactor),
      HermiteInterpolate(lminusoneParts.k.z, lParts.k.z, rParts.k.z,
                         rplusoneParts.k.z, interpFactor));

  // Interpolate the rotation part.
  rParts.q.MakeClosest(lParts.q);
  Quat interpRot = Slerp(lParts.q, rParts.q, interpFactor);

  Matrix3 interpMat(1);

  interpMat.SetScale(interpScale);

  Matrix3 rotMat(1);
  rotMat.SetRotate(interpRot);

  interpMat = interpMat * rotMat;

  interpMat.Translate(interpPos);

  return interpMat;
}

// ----------------------------------------------------------------------------

BOOL ParticleTrailSource::isTrailStationary(
    int trailIdx, TimeValue startT, TimeValue endT, TimeValue keyStep,
    std::vector<TimeValue>& levelTimes) {
  // Return TRUE if the trail is stationary over the interval startT->endT,
  // FALSE otherwise. This is used to cull zero-length trails from mesh
  // generation. Sample the motion every keyStep ticks.

  if (!pPartState) return TRUE;

  ParticleState& particle = pPartState->getParticle(trailIdx);

  // Do (startT, endT) and (particle->getBirthTime(), particle->getDeathTime())
  // intersect?
  Interval partInterval(particle.getBirthTime(), particle.getDeathTime());
  Interval qryInterval(startT, endT);
  Interval intersection = partInterval & qryInterval;

  if (intersection.Empty() == TRUE) {
    // No overlap - outside particle lifetime so no motion
    return TRUE;
  }

  // If we overlap the particle's lifetime let's find out
  // whether it moves (we're only checking the start and end times,
  // which should be good enough - particle don't usually retrace
  // their steps).
  Matrix3 startTM = getTrailTM(trailIdx, startT, levelTimes);
  Matrix3 endTM = getTrailTM(trailIdx, endT, levelTimes);

  if (Length(startTM.GetTrans() - endTM.GetTrans()) < 0.001f)
    return TRUE;
  else
    return FALSE;
}

// ----------------------------------------------------------------------------

// Get the number of frames for which the particle is stationary. This is
// just equal to the length of the trail minus the age of the particle (ie
// the particle is stationary before it is born
int ParticleTrailSource::getNumStationaryLevels(
    int trailIdx, std::vector<TimeValue>& levelTimes) {
  int count = 0;
  Matrix3 lastTM;
  Matrix3 curTM;

  for (int level = (levelTimes.size() - 1); level > 0; --level) {
    if (level == (levelTimes.size() - 1)) {
      lastTM = getTrailTM(trailIdx, levelTimes[level], levelTimes);
    } else {
      curTM = getTrailTM(trailIdx, levelTimes[level], levelTimes);
      if (curTM.Equals(lastTM)) {
        // No change
        count++;
        lastTM = curTM;
      } else
        return count;
    }
  }

  return 0;
}

// ----------------------------------------------------------------------------

// Get the age of the trail (in ticks) at time t.
// Return -1 to indicate outside the trail's lifetime.
//
TimeValue ParticleTrailSource::getTrailAgeAtTime(int trailIdx, TimeValue t) {
  if (!pPartState) return -1;

  const ParticleState& particle = pPartState->getParticle(trailIdx);

  if ((t >= particle.getBirthTime()) && (t <= particle.getDeathTime()))
    return t - particle.getBirthTime();
  else
    return -1;
}

// ----------------------------------------------------------------------------

// Get the trail lifetime of trail trailIdx
//
TimeValue ParticleTrailSource::getTrailLifeTime(int trailIdx) {
  if (!pPartState) return -1;

  const ParticleState& particle = pPartState->getParticle(trailIdx);

  return particle.getDeathTime() - particle.getBirthTime();
}

// ----------------------------------------------------------------------------

int ParticleTrailSource::getTrailMtlID(int trailIdx) {
  if (!pPartState) {
    return 1;
  }

  const ParticleState& particle = pPartState->getParticle(trailIdx);
  return particle.getMtlID();
}
