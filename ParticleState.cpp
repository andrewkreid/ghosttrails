#include "ParticleState.h"
#include "Max.h"
#include "spline3d.h"
#include "splshape.h"

#ifndef GTASSERT
#define GTASSERT
#endif

// ---------------------------------------------------------------------------
//
// ParticleState.cpp
//
// Classes to hold an animation snapshot of a particle system over a
// certain time period.
//
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//
// class ParticlePositionKey
//
// ---------------------------------------------------------------------------

ParticlePositionKey::ParticlePositionKey() : tm(1), time(0), age(0) {}

// ---------------------------------------------------------------------------

ParticlePositionKey::ParticlePositionKey(const TimeValue& tim, const Point3& pt,
                                         const TimeValue& newAge)
    : time(tim), age(newAge) {
  setPos(pt);
}

// ---------------------------------------------------------------------------

ParticlePositionKey::ParticlePositionKey(const TimeValue& tim,
                                         const Matrix3& newTM,
                                         const TimeValue& newAge)
    : tm(newTM), time(tim), age(newAge) {}

// ---------------------------------------------------------------------------

const Point3 ParticlePositionKey::getPos() const {
  // return pos;
  return tm.GetRow(3);
}

// ---------------------------------------------------------------------------

void ParticlePositionKey::setPos(const Point3& pt) {
  tm = TransMatrix(pt);
  // pos = pt;
}

// ---------------------------------------------------------------------------

TimeValue ParticlePositionKey::getTime() const { return time; }

// ---------------------------------------------------------------------------

void ParticlePositionKey::setTime(TimeValue t) { time = t; }

// ---------------------------------------------------------------------------

const Matrix3& ParticlePositionKey::getTM() const { return tm; }

// ---------------------------------------------------------------------------

void ParticlePositionKey::setTM(const Matrix3& newTM) { tm = newTM; }

// ---------------------------------------------------------------------------

TimeValue ParticlePositionKey::getAge() const { return age; }

// ---------------------------------------------------------------------------

void ParticlePositionKey::setAge(const TimeValue& newAge) { age = newAge; }

// ---------------------------------------------------------------------------
//
// class ParticleState
//
// ---------------------------------------------------------------------------

ParticleState::ParticleState() : birthTime_(0), deathTime_(0), mtlID_(0) {}

// ---------------------------------------------------------------------------

ParticleState::ParticleState(const TimeValue& t) { birthTime_ = t; }

// ---------------------------------------------------------------------------

TimeValue ParticleState::getBirthTime() const { return birthTime_; }

// ---------------------------------------------------------------------------

void ParticleState::setBirthTime(const TimeValue& t) { birthTime_ = t; }

// ---------------------------------------------------------------------------

TimeValue ParticleState::getDeathTime() const { return deathTime_; }

// ---------------------------------------------------------------------------

void ParticleState::setDeathTime(const TimeValue& t) { deathTime_ = t; }

// ---------------------------------------------------------------------------

ParticlePositionKey& ParticleState::getKey(int idx) {
  GTASSERT((idx >= 0) && (idx < keys_.size()));

  return keys_[idx];
}

// ---------------------------------------------------------------------------

void ParticleState::addKey(const ParticlePositionKey& key) {
  keys_.push_back(key);

  // keep the birth and death times up to date to reflect
  // the range of keys provided
  if (keys_.size() == 1) {
    birthTime_ = deathTime_ = key.getTime();
  } else {
    if (key.getTime() > deathTime_) deathTime_ = key.getTime();
    if (key.getTime() < birthTime_) birthTime_ = key.getTime();
  }
}

// ---------------------------------------------------------------------------

void ParticleState::clearKeys() { keys_.clear(); }

// ---------------------------------------------------------------------------

void ParticleState::deleteKey(int idx) {
  GTASSERT((idx >= 0) && (idx < keys_.size()));
  std::vector<ParticlePositionKey>::iterator it = keys_.begin();
  it += idx;
  keys_.erase(it);
}

// ---------------------------------------------------------------------------

int ParticleState::numKeys() const { return keys_.size(); }

// ---------------------------------------------------------------------------

// Is the particle alive at time t?
//
BOOL ParticleState::isAliveAtTime(const TimeValue& t) {
  if ((t >= birthTime_) && (t <= deathTime_))
    return TRUE;
  else
    return FALSE;
}

int ParticleState::getMtlID() const { return mtlID_; }

void ParticleState::setMtlID(int mtlID) { mtlID_ = mtlID; }

// ---------------------------------------------------------------------------
//
// class ParticleSystemState
//
// ---------------------------------------------------------------------------

ParticleSystemState::ParticleSystemState()
    : startTime(0),
      endTime(1),
      psType(ParticleSystemState::PS_STANDARD),
      maximumAge(-1),
      stepTime(40) {}

// ---------------------------------------------------------------------------

ParticleSystemState::ParticleSystemState(const TimeValue& start,
                                         const TimeValue& end) {
  setTimePeriod(start, end);
}

// ---------------------------------------------------------------------------

TimeValue ParticleSystemState::getStartTime() const { return startTime; }

// ---------------------------------------------------------------------------

void ParticleSystemState::setStartTime(const TimeValue& time) {
  GTASSERT(time < endTime);
  startTime = time;
}

// ---------------------------------------------------------------------------

TimeValue ParticleSystemState::getEndTime() const { return endTime; }

void ParticleSystemState::setEndTime(const TimeValue& time) {
  GTASSERT(startTime < time);
  endTime = time;
}

// ---------------------------------------------------------------------------

void ParticleSystemState::setTimePeriod(const TimeValue& start,
                                        const TimeValue& end) {
  GTASSERT(start < end);

  startTime = start;
  endTime = end;
}

// ---------------------------------------------------------------------------

int ParticleSystemState::numParticles() const { return particles.size(); }

// ---------------------------------------------------------------------------

ParticleState& ParticleSystemState::getParticle(int idx) {
  GTASSERT((idx >= 0) && (idx < particles.size()));
  return particles[idx];
}

// ---------------------------------------------------------------------------

void ParticleSystemState::addParticle(const ParticleState& pState) {
  particles.push_back(pState);
}

// ---------------------------------------------------------------------------

void ParticleSystemState::clearParticles() {
  particles.clear();
  maximumAge = -1;
}

// ---------------------------------------------------------------------------

void ParticleSystemState::removeParticle(int idx) {
  GTASSERT((idx >= 0) && (idx < particles.size()));
  particles.erase(particles.begin() + idx);
}

// ---------------------------------------------------------------------------
//
// Resize the particle state array
//
void ParticleSystemState::setNumParticles(int siz) { particles.resize(siz); }

// ---------------------------------------------------------------------------

ParticleSystemState::ParticleSystemType
ParticleSystemState::getSourceSystemType() const {
  return psType;
}

// ---------------------------------------------------------------------------

void ParticleSystemState::setSourceSystemType(ParticleSystemType newType) {
  psType = newType;
}

// ---------------------------------------------------------------------------

// Test method to create a spline from the particle paths
// (so we can easily check it's working visually)
//
void ParticleSystemState::makeTestSpline(std::wstring splineName) {
  Interface* ip = GetCOREInterface();

  SplineShape* splShape = new SplineShape;

  if (!splShape) {
    DebugPrint(
        _T("ERROR: ParticleSystemState::makeTestSpline couldn't create spline ")
        _T("object\n"));
    return;
  }

  BezierShape& bezShape = splShape->GetShape();
  bezShape.NewShape();

  // add a spline for each particle track
  for (int i = 0; i < numParticles(); i++) {
    ParticleState& pstate = getParticle(i);

    Spline3D* spline = bezShape.NewSpline();
    if (!spline) continue;

    // ...with a knot for each key
    for (int j = 0; j < pstate.numKeys(); j++) {
      ParticlePositionKey& pkey = pstate.getKey(j);

      // TODO - make spline points (see 3dsimp.cpp)
      SplineKnot knot(KTYPE_AUTO, LTYPE_CURVE, pkey.getPos(), pkey.getPos(),
                      pkey.getPos());
      spline->AddKnot(knot);
    }
    // spline->SetClosed();
  }

  // init the selection info.
  bezShape.UpdateSels();

  // create the node to put the object in the scene
  INode* splineNode = ip->CreateObjectNode(splShape);
  splineNode->SetName(
      const_cast<TCHAR*>(static_cast<const TCHAR*>(splineName.data())));
}

// ---------------------------------------------------------------------------

TimeValue ParticleSystemState::getStepTime() const { return stepTime; }

// ---------------------------------------------------------------------------

void ParticleSystemState::setStepTime(const TimeValue& newTime) {
  stepTime = newTime;
}

// ---------------------------------------------------------------------------

TimeValue ParticleSystemState::getMaximumParticleAge() {
  if (maximumAge == -1) {
    calculateMaximumParticleAge();
  }

  return maximumAge;
}

// ---------------------------------------------------------------------------

void ParticleSystemState::calculateMaximumParticleAge() {
  maximumAge = -1;
  for (int i = 0; i < numParticles(); i++) {
    ParticleState& pstate = getParticle(i);
    TimeValue curTime = pstate.getDeathTime() - pstate.getBirthTime();
    if (curTime > maximumAge) maximumAge = curTime;
  }
}

// ---------------------------------------------------------------------------
//
// Save format constants
//

static const int formatVersion = 1;
static const int requiredVersion = 1;

static const int PARTICLE_STATE_CHUNK = 1000;
static const int PARTICLE_STATE_MTLID_CHUNK = 1001;

// ---------------------------------------------------------------------------
//
// Save the particle system state to a .MAX scene file
//
IOResult ParticleSystemState::save(ISave* isave) {
  ULONG nb;

  isave->BeginChunk(PARTICLE_STATE_CHUNK);

  // write the format version number
  if (isave->Write(&formatVersion, sizeof(int), &nb) != IO_OK) return IO_ERROR;

  if (isave->Write(&startTime, sizeof(TimeValue), &nb) != IO_OK)
    return IO_ERROR;

  if (isave->Write(&endTime, sizeof(TimeValue), &nb) != IO_OK) return IO_ERROR;

  if (isave->Write(&stepTime, sizeof(TimeValue), &nb) != IO_OK) return IO_ERROR;

  if (isave->Write(&maximumAge, sizeof(TimeValue), &nb) != IO_OK)
    return IO_ERROR;

  int pstI = static_cast<int>(psType);
  if (isave->Write(&pstI, sizeof(int), &nb) != IO_OK) return IO_ERROR;

  // write each particle.
  int numParticles = particles.size();
  if (isave->Write(&numParticles, sizeof(int), &nb) != IO_OK) return IO_ERROR;

  for (int i = 0; i < numParticles; i++) {
    ParticleState& partstate = particles[i];

    TimeValue birthTime = partstate.getBirthTime();
    if (isave->Write(&birthTime, sizeof(TimeValue), &nb) != IO_OK)
      return IO_ERROR;

    TimeValue deathTime = partstate.getDeathTime();
    if (isave->Write(&deathTime, sizeof(TimeValue), &nb) != IO_OK)
      return IO_ERROR;

    int numKeys = partstate.numKeys();
    if (isave->Write(&numKeys, sizeof(int), &nb) != IO_OK) return IO_ERROR;

    // write each key for a particle.
    for (int k = 0; k < numKeys; k++) {
      ParticlePositionKey& pkey = partstate.getKey(k);

      const Matrix3& tm = pkey.getTM();
      if (isave->Write(&tm, sizeof(Matrix3), &nb) != IO_OK) return IO_ERROR;

      TimeValue t = pkey.getTime();
      if (isave->Write(&t, sizeof(TimeValue), &nb) != IO_OK) return IO_ERROR;

      TimeValue age = pkey.getAge();
      if (isave->Write(&age, sizeof(TimeValue), &nb) != IO_OK) return IO_ERROR;
    }
  }

  isave->EndChunk();

  // Save per-trails material IDs (new in 3dsmax2018)
  isave->BeginChunk(PARTICLE_STATE_MTLID_CHUNK);
  if (isave->Write(&numParticles, sizeof(int), &nb) != IO_OK) return IO_ERROR;
  for (int i = 0; i < numParticles; i++) {
	  ParticleState& partstate = particles[i];
	  int mtlID = partstate.getMtlID();
	  if (isave->Write(&mtlID, sizeof(int), &nb) != IO_OK) return IO_ERROR;
  }
  isave->EndChunk();

  return IO_OK;
}

// ---------------------------------------------------------------------------
//
// Load the particle system state from a .MAX file
//
IOResult ParticleSystemState::load(ILoad* iload) {
  clearParticles();

  ULONG nb;
  IOResult res;

  while (IO_OK == (res = iload->OpenChunk())) {
    switch (iload->CurChunkID()) {
      case PARTICLE_STATE_CHUNK: {
        int dataVersion;
        if (iload->Read(&dataVersion, sizeof(int), &nb) != IO_OK)
          return IO_ERROR;

        // Got a version we understand?
        if (dataVersion != requiredVersion) {
          // No. Just clear and return.
          clearParticles();
          iload->CloseChunk();
          return IO_OK;
        }

        if (iload->Read(&startTime, sizeof(TimeValue), &nb) != IO_OK)
          return IO_ERROR;

        if (iload->Read(&endTime, sizeof(TimeValue), &nb) != IO_OK)
          return IO_ERROR;

        if (iload->Read(&stepTime, sizeof(TimeValue), &nb) != IO_OK)
          return IO_ERROR;

        if (iload->Read(&maximumAge, sizeof(TimeValue), &nb) != IO_OK)
          return IO_ERROR;

        int pstI = 0;
        if (iload->Read(&pstI, sizeof(int), &nb) != IO_OK) return IO_ERROR;
        psType = static_cast<ParticleSystemType>(pstI);

        int numParticles = 0;
        if (iload->Read(&numParticles, sizeof(int), &nb) != IO_OK)
          return IO_ERROR;

        for (int i = 0; i < numParticles; i++) {
          ParticleState partstate;

          TimeValue birthTime;
          if (iload->Read(&birthTime, sizeof(TimeValue), &nb) != IO_OK)
            return IO_ERROR;
          partstate.setBirthTime(birthTime);

          TimeValue deathTime;
          if (iload->Read(&deathTime, sizeof(TimeValue), &nb) != IO_OK)
            return IO_ERROR;
          partstate.setDeathTime(deathTime);

          int numKeys;
          if (iload->Read(&numKeys, sizeof(int), &nb) != IO_OK) return IO_ERROR;

          // read each key for a particle.
          for (int k = 0; k < numKeys; k++) {
            Matrix3 tm;
            if (iload->Read(&tm, sizeof(Matrix3), &nb) != IO_OK)
              return IO_ERROR;

            TimeValue t;
            if (iload->Read(&t, sizeof(TimeValue), &nb) != IO_OK)
              return IO_ERROR;

            TimeValue age;
            if (iload->Read(&age, sizeof(TimeValue), &nb) != IO_OK)
              return IO_ERROR;

            partstate.addKey(ParticlePositionKey(t, tm, age));
          }

          addParticle(partstate);
        }

        iload->CloseChunk();
      } break;
	  case PARTICLE_STATE_MTLID_CHUNK: {
		  // Read material IDs for the particles. We should already have read the other
		  // particle info.
		  int numParticles = 0;
		  if (iload->Read(&numParticles, sizeof(int), &nb) != IO_OK)
			  return IO_ERROR;
		  if (numParticles != this->numParticles())
			  return IO_ERROR;
		  for (int i = 0; i < numParticles; i++) {
			  ParticleState& partState = getParticle(i);
			  int mtlID = 0;
			  if (iload->Read(&mtlID, sizeof(int), &nb) != IO_OK) return IO_ERROR;
			  partState.setMtlID(mtlID);
		  }
		  iload->CloseChunk();
	  } break;
      default:
        iload->CloseChunk();
        break;
    }
  }

  return IO_OK;
}
