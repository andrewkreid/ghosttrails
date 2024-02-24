#include "Max.h"
#include <string>

// ---------------------------------------------------------------------------

class MaxUtils {
 public:
  static INode* GetNodeReference(ReferenceMaker* reference_target);

  // Given a modifier, find the INode it's applied to.
  static INode* FindNodeReference(ReferenceTarget* reference_target);
  static INode* FindNodeReference(ReferenceTarget* modifier,
                                  ModContext* mod_context);

  static bool fpEquals(double val, double val2, double EPSILON = 0.000001);

  // Convert wchar_t* to char*. Caller must delete returned string.
  static char* wcharToChar(const wchar_t* orig);

  static std::wstring stringToWstring(const std::string& str);

};

// ---------------------------------------------------------------------------
