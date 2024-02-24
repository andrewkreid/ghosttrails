#ifndef _PARTICLESTATE_H_
#define _PARTICLESTATE_H_

#include <vector>
#include <max.h>

// ---------------------------------------------------------------------------
//
// ParticleState.h
//
// Classes to hold an animation snapshot of a particle system over a
// certain time period.
//
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//
// class ParticlePosition
//
//   A position key for a particle
//
// ---------------------------------------------------------------------------

class ParticlePositionKey {
 public:
  ParticlePositionKey();
  ParticlePositionKey(const TimeValue&, const Point3&, const TimeValue&);
  ParticlePositionKey(const TimeValue&, const Matrix3&, const TimeValue&);

  // The position of the particle (TODO world or object space?)
  const Point3 getPos() const;
  void setPos(const Point3&);

  const Matrix3& getTM() const;
  void setTM(const Matrix3&);

  // The time (max ticks) for the key
  TimeValue getTime() const;
  void setTime(TimeValue);

  // The age of the particle at the time of the keyframe
  TimeValue getAge() const;
  void setAge(const TimeValue&);

 private:
  // Point3		pos;
  Matrix3 tm;
  TimeValue time;
  TimeValue age;
};

// ---------------------------------------------------------------------------
//
// class ParticleState
//
//   Holds a snapshot of the state of a particular particle over a time
//   period
//
// ---------------------------------------------------------------------------

class ParticleState {
 public:
  ParticleState();
  ParticleState(const TimeValue&);

  TimeValue getBirthTime() const;
  void setBirthTime(const TimeValue&);
  TimeValue getDeathTime() const;
  void setDeathTime(const TimeValue&);

  BOOL isAliveAtTime(const TimeValue&);

  int numKeys() const;
  ParticlePositionKey& getKey(int idx);
  void addKey(const ParticlePositionKey&);
  void clearKeys();
  void deleteKey(int idx);

  int getMtlID() const;
  void setMtlID(int mtlID);

 private:
  // The time the particle is born.
  TimeValue birthTime_;

  // The time the particle dies (== time of last animation key).
  TimeValue deathTime_;

  // The animation keys (linear interpolation is assumed)
  std::vector<ParticlePositionKey> keys_;

  // Material ID of the particle (only set for PS_PARTICLEFLOW), 1 otherwise.
  int mtlID_;
};

// ---------------------------------------------------------------------------
//
// class ParticleSystemState
//
//   Holds a snapshot of a particle system over a certain time period. In
//   other words, ParticleSystemState contains an array of particles and their
//   animation data.
//
// ---------------------------------------------------------------------------

class ParticleSystemState {
 public:
  // Types of 3dsmax particle system
  enum ParticleSystemType {
    PS_STANDARD = 0,  // Spray, Blizzard, Snow etc.
    PS_PARTICLEFLOW,  // Particle Flow (3dsmax6 +)

    PS_NUMBERSYSTEMS
  };

  ParticleSystemState();
  ParticleSystemState(const TimeValue& start, const TimeValue& end);

  TimeValue getStartTime() const;
  void setStartTime(const TimeValue&);

  TimeValue getEndTime() const;
  void setEndTime(const TimeValue&);

  TimeValue getStepTime() const;
  void setStepTime(const TimeValue&);

  TimeValue getMaximumParticleAge();

  void setTimePeriod(const TimeValue& start, const TimeValue& end);

  int numParticles() const;
  ParticleState& getParticle(int idx);
  void addParticle(const ParticleState&);
  void clearParticles();
  void removeParticle(int idx);

  void setNumParticles(int);

  ParticleSystemType getSourceSystemType() const;
  void setSourceSystemType(ParticleSystemType);

  // Test method to create a spline from the particle paths
  // (so we can easily check it's working visually)
  void makeTestSpline(std::wstring splineName = std::wstring(L"TestParticleSpline"));

  void calculateMaximumParticleAge();

  IOResult save(ISave*);
  IOResult load(ILoad*);

  void setParticleMtlID(int idx, int mtlID);
  int getParticleMtlID(int idx);

 private:
  ParticleSystemState(const ParticleSystemState&);
  ParticleSystemState& operator=(const ParticleSystemState&);

  // The time at which the sampling of particle positions started.
  TimeValue startTime;

  // The time at which the sampling of particle positions ended.
  TimeValue endTime;

  // Sampling rate
  TimeValue stepTime;

  // The maximum age of the longest lived particle in the state.
  TimeValue maximumAge;

  // The particles
  std::vector<ParticleState> particles;

  // What sort of particle system did we come from
  ParticleSystemType psType;
};

// ---------------------------------------------------------------------------

#endif  // _PARTICLESTATE_H_
