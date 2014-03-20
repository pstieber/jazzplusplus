#include "JazzPlusPlusApplication.h"

int GetOptionIndex(const wxString& Option)
{
  int ArgumentIndex = 0;

  for (int Index = 1; Index < ::wxGetApp().argc; ++Index)
  {
    if (::wxGetApp().argv[Index] == Option)
    {
      ArgumentIndex = Index;
    }
  }
  return ArgumentIndex;
}
