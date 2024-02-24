
#include "Max.h"
#include "modstack.h"

#include "MaxUtils.h"

INode *MaxUtils::GetNodeReference(ReferenceMaker *rm) {
  if (!rm) return 0;

  if (rm->SuperClassID() == BASENODE_CLASS_ID)
    return (INode *)rm;
  else
    return rm->IsRefTarget() ? FindNodeReference((ReferenceTarget *)rm) : NULL;
}

// ---------------------------------------------------------------------------

INode *MaxUtils::FindNodeReference(ReferenceTarget *reference_target) {
  DependentIterator di(reference_target);
  ReferenceMaker *reference_maker;
  INode *node = NULL;
  while (reference_maker = di.Next()) {
    node = GetNodeReference(reference_maker);
    if (node) return node;
  }
  return NULL;
}

// ---------------------------------------------------------------------------

// Find the node corresponding to the modifier and modcontext
//
INode *MaxUtils::FindNodeReference(ReferenceTarget *modifier,
                                   ModContext *mod_context) {
  DependentIterator di(modifier);
  ReferenceMaker *reference_maker;
  while (reference_maker = di.Next()) {
    if (reference_maker->SuperClassID() == BASENODE_CLASS_ID) {
      INode *node = (INode *)reference_maker;
      Object *object = node->GetObjectRef();

      // Get modifier and modcontext and see which one matches
      if (object) {
        IDerivedObject *derived_object = (IDerivedObject *)object;

        int modifier_index;
        for (modifier_index = 0;
             modifier_index < derived_object->NumModifiers();
             modifier_index++) {
          Modifier *modifier = derived_object->GetModifier(modifier_index);
          ModContext *modifier_context =
              derived_object->GetModContext(modifier_index);

          if (modifier_context == mod_context) return node;
        }
      }
    }
    if (reference_maker->IsRefTarget()) {
      INode *node =
          FindNodeReference((ReferenceTarget *)reference_maker, mod_context);

      if (node) return node;
    }
  }

  return NULL;
}

// ---------------------------------------------------------------------------

bool MaxUtils::fpEquals(double val, double val2, double EPSILON) {
  if (fabs(val - val2) < EPSILON)
    return true;
  else
    return false;
}

// ---------------------------------------------------------------------------

char *MaxUtils::wcharToChar(const wchar_t *orig) {
  // This code from https://msdn.microsoft.com/en-us/library/ms235631.aspx

  // Convert the wchar_t string to a char* string. Record
  //.the length of the original string and add 1 to it to
  //.account for the terminating null character.
  size_t origsize = wcslen(orig) + 1;
  size_t convertedChars = 0;

  // Allocate two bytes in the multibyte output string for every wide
  // character in the input string (including a wide character
  // null). Because a multibyte character can be one or two bytes,
  // you should allot two bytes for each character. Having extra
  // space for the new string is not an error, but having
  // insufficient space is a potential security problem.
  const size_t newsize = origsize * 2;
  // The new string will contain a converted copy of the original
  // string.
  char *nstring = new char[newsize];

  // Put a copy of the converted string into nstring
  wcstombs_s(&convertedChars, nstring, newsize, orig, _TRUNCATE);
  // Display the result.
  return nstring;
}

std::wstring MaxUtils::stringToWstring(const std::string& str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

