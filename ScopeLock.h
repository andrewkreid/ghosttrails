// **************************************************************************

//
// ScopeLock
//
// A class whose purpose is to provide an easy way to manage
// the acquisition and release of a CRITICAL_SECTION object
//
// The critical section is entered in the constructor and
// left in the destructor. This is useful for scope based
// synchronization. For example
//
// void f()
// {
//    ScopeLock(&CSObject);
//
//    // do some stuff
//
// }  // critical section released on block exit.
//
class ScopeLock {
 public:
  ScopeLock(CRITICAL_SECTION*);
  ~ScopeLock();

 private:
  CRITICAL_SECTION* m_pCritSec;

  // disable copy ctor and assignent operator.
  ScopeLock(const ScopeLock&);
  ScopeLock& operator=(const ScopeLock);
};

// **************************************************************************
