/////////////////////////////////////////////////////////////////////////////
// Name:        prop.cpp
// Purpose:     Propert sheet classes implementation
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// RCS-ID:      $Id: prop.cpp 38724 2006-04-14 19:56:03Z ABX $
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "setup.h"

#if wxUSE_PROPSHEET

#ifndef WX_PRECOMP
#endif

#include "wx/debug.h"
#include "wx/wxcrtvararg.h"
#include "prop.h"

#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

using namespace std;

#if !WXWIN_COMPATIBILITY_2_4
static inline wxChar* copystring(const wxChar* s)
    { return wxStrcpy(new wxChar[wxStrlen(s) + 1], s); }
#endif

IMPLEMENT_DYNAMIC_CLASS(wxPropertyValue, wxObject)

wxPropertyValue::wxPropertyValue(void)
{
  m_type = wxPropertyValueNull;
  m_next = NULL;
  m_last = NULL;
  m_value.first = NULL;
  m_clientData = NULL;
  m_modifiedFlag = false;
}

wxPropertyValue::wxPropertyValue(const wxPropertyValue& copyFrom)
    : wxObject()
{
  m_value.string = (wxChar*) NULL;
  m_modifiedFlag = false;
  Copy((wxPropertyValue& )copyFrom);
}

wxPropertyValue::wxPropertyValue(const wxChar *val)
{
  m_modifiedFlag = false;
  m_type = wxPropertyValueString;

  m_value.string = copystring(val);
  m_clientData = NULL;
  m_next = NULL;
  m_last = NULL;
}

wxPropertyValue::wxPropertyValue(const wxString& val)
{
  m_modifiedFlag = false;
  m_type = wxPropertyValueString;

  m_value.string = copystring(val.c_str());
  m_clientData = NULL;
  m_next = NULL;
  m_last = NULL;
}

wxPropertyValue::wxPropertyValue(long the_integer)
{
  m_modifiedFlag = false;
  m_type = wxPropertyValueInteger;
  m_value.integer = the_integer;
  m_clientData = NULL;
  m_next = NULL;
}

wxPropertyValue::wxPropertyValue(bool val)
{
  m_modifiedFlag = false;
  m_type = wxPropertyValuebool;
  m_value.integer = val;
  m_clientData = NULL;
  m_next = NULL;
}

wxPropertyValue::wxPropertyValue(float the_real)
{
  m_modifiedFlag = false;
  m_type = wxPropertyValueReal;
  m_value.real = the_real;
  m_clientData = NULL;
  m_next = NULL;
}

wxPropertyValue::wxPropertyValue(double the_real)
{
  m_modifiedFlag = false;
  m_type = wxPropertyValueReal;
  m_value.real = (float)the_real;
  m_clientData = NULL;
  m_next = NULL;
}

// Pointer versions: we have a pointer to the real C++ value.
wxPropertyValue::wxPropertyValue(wxChar **val)
{
  m_modifiedFlag = false;
  m_type = wxPropertyValueStringPtr;

  m_value.stringPtr = val;
  m_clientData = NULL;
  m_next = NULL;
  m_last = NULL;
}

wxPropertyValue::wxPropertyValue(int *val)
{
  m_modifiedFlag = false;
  m_type = wxPropertyValueIntegerPtr;
  m_value.integerPtr = val;
  m_clientData = NULL;
  m_next = NULL;
}

wxPropertyValue::wxPropertyValue(long *val)
{
  m_modifiedFlag = false;
  m_type = wxPropertyValueLongPtr;
  m_value.longPtr = val;
  m_clientData = NULL;
  m_next = NULL;
}

wxPropertyValue::wxPropertyValue(bool *val)
{
  m_modifiedFlag = false;
  m_type = wxPropertyValueboolPtr;
  m_value.boolPtr = val;
  m_clientData = NULL;
  m_next = NULL;
}

wxPropertyValue::wxPropertyValue(float *val)
{
  m_modifiedFlag = false;
  m_type = wxPropertyValueRealPtr;
  m_value.realPtr = val;
  m_clientData = NULL;
  m_next = NULL;
}

wxPropertyValue::wxPropertyValue(double *val)
{
  m_modifiedFlag = false;
  m_type = wxPropertyValueDoublePtr;
  m_value.doublePtr = val;
  m_clientData = NULL;
  m_next = NULL;
}

wxPropertyValue::wxPropertyValue(wxList *the_list)
{
  m_modifiedFlag = false;
  m_type = wxPropertyValueList;
  m_clientData = NULL;
  m_last = NULL;
  m_value.first = NULL;

  wxObjectList::compatibility_iterator node = the_list->GetFirst();
  while (node)
  {
    wxPropertyValue *expr = (wxPropertyValue *)node->GetData();
    Append(expr);
    node = node->GetNext();
  }

  delete the_list;
}

wxPropertyValue::wxPropertyValue(wxStringList *the_list)
{
  m_modifiedFlag = false;
  m_type = wxPropertyValueList;
  m_clientData = NULL;
  m_last = NULL;
  m_value.first = NULL;

  wxStringList::compatibility_iterator node = the_list->GetFirst();
  while (node)
  {
    wxString s = node->GetData();
    Append(new wxPropertyValue(s));
    node = node->GetNext();
  }
  delete the_list;
}

wxPropertyValue::~wxPropertyValue(void)
{
  switch (m_type)
  {
    case wxPropertyValueInteger:
    case wxPropertyValuebool:
    case wxPropertyValueReal:
    {
     break;
    }
   case wxPropertyValueString:
   {
     delete[] m_value.string;
     break;
   }
   case wxPropertyValueList:
   {
     wxPropertyValue *expr = m_value.first;
     while (expr)
     {
       wxPropertyValue *expr1 = expr->m_next;

       delete expr;
       expr = expr1;
     }
     break;
   }
   default:
   case wxPropertyValueNull: break;
  }
}

void wxPropertyValue::Append(wxPropertyValue *expr)
{
  m_modifiedFlag = true;
  if (!m_value.first)
    m_value.first = expr;

  if (m_last)
    m_last->m_next = expr;
  m_last = expr;
}

void wxPropertyValue::Insert(wxPropertyValue *expr)
{
  m_modifiedFlag = true;
  expr->m_next = m_value.first;
  m_value.first = expr;

  if (!m_last)
    m_last = expr;
}

// Delete from list
void wxPropertyValue::Delete(wxPropertyValue *node)
{
  wxPropertyValue *expr = GetFirst();

  wxPropertyValue *previous = NULL;
  while (expr && (expr != node))
  {
    previous = expr;
    expr = expr->GetNext();
  }

  if (expr)
  {
    if (previous)
      previous->m_next = expr->m_next;

    // If node was the first in the list,
    // make the list point to the NEXT one.
    if (GetFirst() == expr)
    {
      m_value.first = expr->m_next;
    }

    // If node was the last in the list,
    // make the list 'last' pointer point to the PREVIOUS one.
    if (GetLast() == expr)
    {
      if (previous)
        m_last = previous;
      else
        m_last = NULL;
    }
    m_modifiedFlag = true;
    delete expr;
  }

}

void wxPropertyValue::ClearList(void)
{
  wxPropertyValue *val = GetFirst();
  if (val)
    m_modifiedFlag = true;

  while (val)
  {
    wxPropertyValue *next = val->GetNext();
    delete val;
    val = next;
  }
  m_value.first = NULL;
  m_last = NULL;
}

wxPropertyValue *wxPropertyValue::NewCopy(void) const
{
  switch (m_type)
  {
    case wxPropertyValueInteger:
      return new wxPropertyValue(m_value.integer);
    case wxPropertyValuebool:
      return new wxPropertyValue((bool) (m_value.integer != 0));
    case wxPropertyValueReal:
      return new wxPropertyValue(m_value.real);
    case wxPropertyValueString:
      return new wxPropertyValue(m_value.string);
    case wxPropertyValueList:
    {
      wxPropertyValue *expr = m_value.first;
      wxPropertyValue *new_list = new wxPropertyValue;
      new_list->SetType(wxPropertyValueList);
      while (expr)
      {
        wxPropertyValue *expr2 = expr->NewCopy();
        new_list->Append(expr2);
        expr = expr->m_next;
      }
      return new_list;
    }
   case wxPropertyValueIntegerPtr:
     return new wxPropertyValue(m_value.integerPtr);
   case wxPropertyValueLongPtr:
     return new wxPropertyValue(m_value.longPtr);
   case wxPropertyValueRealPtr:
     return new wxPropertyValue(m_value.realPtr);
   case wxPropertyValueDoublePtr:
     return new wxPropertyValue(m_value.doublePtr);
   case wxPropertyValueboolPtr:
     return new wxPropertyValue(m_value.boolPtr);
   case wxPropertyValueStringPtr:
     return new wxPropertyValue(m_value.stringPtr);

   case wxPropertyValueNull:
    wxFAIL_MSG( wxT("Should never get here!\n" ) );
    break;
  }
  return NULL;
}

void wxPropertyValue::Copy(wxPropertyValue& copyFrom)
{
  if (m_type == wxPropertyValueString)
  {
    delete[] m_value.string ;
    m_value.string = NULL;
  }
  m_type = copyFrom.Type();

  switch (m_type)
  {
    case wxPropertyValueInteger:
      (*this) = copyFrom.IntegerValue();
      return ;

    case wxPropertyValueReal:
      (*this) = copyFrom.RealValue();
      return ;

    case wxPropertyValueString:
      (*this) = wxString(copyFrom.StringValue());
      return ;

    case wxPropertyValuebool:
      (*this) = copyFrom.BoolValue();
      return ;

    // Pointers
    case wxPropertyValueboolPtr:
      (*this) = copyFrom.BoolValuePtr();
      return ;
    case wxPropertyValueRealPtr:
      (*this) = copyFrom.RealValuePtr();
      return ;
    case wxPropertyValueDoublePtr:
      (*this) = copyFrom.DoubleValuePtr();
      return ;
    case wxPropertyValueIntegerPtr:
      (*this) = copyFrom.IntegerValuePtr();
      return ;
    case wxPropertyValueLongPtr:
      (*this) = copyFrom.LongValuePtr();
      return ;
    case wxPropertyValueStringPtr:
    {
      wxChar** s = copyFrom.StringValuePtr();

#if 0
      // what is this? are you trying to assign a bool or a string?  VA can't figure it out..
#if defined(__VISAGECPP__) || defined( __VISUALC__ )
      (*this) = s;
#else
      (*this) = s != 0;
#endif
#endif // if 0

      (*this) = (bool)(s != 0);

      return ;
    }

    case wxPropertyValueList:
    {
      m_value.first = NULL;
      m_next = NULL;
      m_last = NULL;
      wxPropertyValue *expr = copyFrom.m_value.first;
      while (expr)
      {
        wxPropertyValue *expr2 = expr->NewCopy();
        Append(expr2);
        expr = expr->m_next;
      }
      return;
    }
   case wxPropertyValueNull:
    wxFAIL_MSG( wxT("Should never get here!\n" ) );
    break;
  }
}

// Return nth argument of a clause (starting from 1)
wxPropertyValue *wxPropertyValue::Arg(wxPropertyValueType type, int arg) const
{
  wxPropertyValue *expr = m_value.first;
  for (int i = 1; i < arg; i++)
    if (expr)
      expr = expr->m_next;

  if (expr && (expr->m_type == type))
    return expr;
  else
    return NULL;
}

// Return nth argument of a list expression (starting from zero)
wxPropertyValue *wxPropertyValue::Nth(int arg) const
{
  if (m_type != wxPropertyValueList)
    return NULL;

  wxPropertyValue *expr = m_value.first;
  for (int i = 0; i < arg; i++)
    if (expr)
      expr = expr->m_next;
    else return NULL;

  if (expr)
    return expr;
  else
    return NULL;
}

  // Returns the number of elements in a list expression
int wxPropertyValue::Number(void) const
{
  if (m_type != wxPropertyValueList)
    return 0;

  int i = 0;
  wxPropertyValue *expr = m_value.first;
  while (expr)
  {
    expr = expr->m_next;
    i ++;
  }
  return i;
}

void wxPropertyValue::WritePropertyClause(wxString& stream)  // Write this expression as a top-level clause
{
  if (m_type != wxPropertyValueList)
    return;

  wxPropertyValue *node = m_value.first;
  if (node)
  {
    node->WritePropertyType(stream);
    stream.Append( wxT("(") );
    node = node->m_next;
    bool first = true;
    while (node)
    {
      if (!first)
        stream.Append( wxT("  ") );
      node->WritePropertyType(stream);
      node = node->m_next;
      if (node)
        stream.Append( wxT(",\n" ) );
      first = false;
    }
    stream.Append( wxT(").\n\n") );
  }
}

void wxPropertyValue::WritePropertyType(wxString& stream)    // Write as any other subexpression
{
  wxString tmp;
  switch (m_type)
  {
    case wxPropertyValueInteger:
    {
      tmp.Printf( wxT("%ld"), m_value.integer );
      stream.Append( tmp );
      break;
    }
    case wxPropertyValueIntegerPtr:
    {
      tmp.Printf( wxT("%d"), *m_value.integerPtr );
      stream.Append( tmp );
      break;
    }
    case wxPropertyValueLongPtr:
    {
      tmp.Printf( wxT("%ld"), *m_value.longPtr );
      stream.Append( tmp );
      break;
    }
    case wxPropertyValuebool:
    {
      if (m_value.integer)
        stream.Append( wxT("True") );
      else
        stream.Append( wxT("False") );
      break;
    }
    case wxPropertyValueboolPtr:
    {
      if (*m_value.integerPtr)
        stream.Append( wxT("True") );
      else
        stream.Append( wxT("False") );
      break;
    }
    case wxPropertyValueReal:
    {
      double d = m_value.real;
      tmp.Printf( wxT("%.6g"), d );
      stream.Append( tmp );
      break;
    }
    case wxPropertyValueRealPtr:
    {
      double d = *m_value.realPtr;
      tmp.Printf( wxT("%.6g"), d );
      stream.Append( tmp );
      break;
    }
    case wxPropertyValueDoublePtr:
    {
      double d = *m_value.doublePtr;
      tmp.Printf( wxT("%.6g"), d );
      stream.Append( tmp );
      break;
    }
    case wxPropertyValueString:
    {
      stream.Append( m_value.string );
      break;
    }
    case wxPropertyValueStringPtr:
    {
      wxFAIL_MSG( wxT("wxPropertyValue::WritePropertyType( wxPropertyValueStringPtr ) not implemented") );
      /*
      int i;
      int len = strlen(*(m_value.stringPtr));
      for (i = 0; i < len; i++)
      {
        char ch = *(m_value.stringPtr)[i];

      }
      */
      break;
    }
    case wxPropertyValueList:
    {
      if (!m_value.first)
        stream.Append( wxT("[]") );
      else
      {
        wxPropertyValue *expr = m_value.first;

        stream.Append( wxT("[") );
        while (expr)
        {
          expr->WritePropertyType(stream);
          expr = expr->m_next;
          if (expr)
        stream.Append( wxT(", ") );
        }
        stream.Append( wxT("]") );
      }
      break;
    }
   case wxPropertyValueNull: break;
  }
}

wxString wxPropertyValue::GetStringRepresentation(void)
{
  wxString str;
  WritePropertyType(str);
  return str;
}

void wxPropertyValue::operator=(const wxPropertyValue& val)
{
  m_modifiedFlag = true;
  Copy((wxPropertyValue&)val);
}

// void wxPropertyValue::operator=(const char *val)
void wxPropertyValue::operator=(const wxString& val1)
{
  const wxChar *val = val1.c_str();

  m_modifiedFlag = true;

  wxPropertyValueType oldType = m_type;
  if (oldType == wxPropertyValueString)
  {
    delete[] m_value.string ;
    m_value.string = NULL;
  }

  if (m_type == wxPropertyValueNull)
    m_type = wxPropertyValueString;

  if (m_type == wxPropertyValueString)
  {
    if (val)
      m_value.string = copystring(val);
    else
      m_value.string = NULL;
  }
  else if (m_type == wxPropertyValueStringPtr)
  {
    wxFAIL_MSG( wxT("Shouldn't try to assign a wxString reference to a char* pointer.") );
    if (val)
      *m_value.stringPtr = copystring(val);
    else
      *m_value.stringPtr = NULL;
  }

  m_clientData = NULL;
  m_next = NULL;
  m_last = NULL;

}

void wxPropertyValue::operator=(const long val)
{
  wxPropertyValueType oldType = m_type;
  if (oldType == wxPropertyValueString)
  {
    delete[] m_value.string ;
    m_value.string = NULL;
  }

  m_modifiedFlag = true;
  if (m_type == wxPropertyValueNull)
    m_type = wxPropertyValueInteger;

  if (m_type == wxPropertyValueInteger)
    m_value.integer = val;
  else if (m_type == wxPropertyValueIntegerPtr)
    *m_value.integerPtr = val;
  else if (m_type == wxPropertyValueLongPtr)
    *m_value.longPtr = val;
  else if (m_type == wxPropertyValueReal)
    m_value.real = (float)val;
  else if (m_type == wxPropertyValueRealPtr)
    *m_value.realPtr = (float)val;
  else if (m_type == wxPropertyValueDoublePtr)
    *m_value.doublePtr = (double)val;

  m_clientData = NULL;
  m_next = NULL;
}

void wxPropertyValue::operator=(const bool val)
{
  wxPropertyValueType oldType = m_type;
  if (oldType == wxPropertyValueString)
  {
    delete[] m_value.string ;
    m_value.string = NULL;
  }

  m_modifiedFlag = true;
  if (m_type == wxPropertyValueNull)
    m_type = wxPropertyValuebool;

  if (m_type == wxPropertyValuebool)
    m_value.integer = (long)val;
  else if (m_type == wxPropertyValueboolPtr)
    *m_value.boolPtr = val;

  m_clientData = NULL;
  m_next = NULL;
}

void wxPropertyValue::operator=(const float val)
{
  wxPropertyValueType oldType = m_type;
  if (oldType == wxPropertyValueString)
  {
    delete[] m_value.string ;
    m_value.string = NULL;
  }

  m_modifiedFlag = true;
  if (m_type == wxPropertyValueNull)
    m_type = wxPropertyValueReal;

  if (m_type == wxPropertyValueInteger)
    m_value.integer = (long)val;
  else if (m_type == wxPropertyValueIntegerPtr)
    *m_value.integerPtr = (int)val;
  else if (m_type == wxPropertyValueLongPtr)
    *m_value.longPtr = (long)val;
  else if (m_type == wxPropertyValueReal)
    m_value.real = val;
  else if (m_type == wxPropertyValueRealPtr)
    *m_value.realPtr = val;
  else if (m_type == wxPropertyValueDoublePtr)
    *m_value.doublePtr = val;

  m_clientData = NULL;
  m_next = NULL;
}

void wxPropertyValue::operator=(const wxChar **val)
{
  wxPropertyValueType oldType = m_type;
  if (oldType == wxPropertyValueString)
  {
    delete[] m_value.string ;
    m_value.string = NULL;
  }

  m_modifiedFlag = true;
  m_type = wxPropertyValueStringPtr;

  if (val)
    m_value.stringPtr = (wxChar **)val;
  else
    m_value.stringPtr = NULL;
  m_clientData = NULL;
  m_next = NULL;
  m_last = NULL;

}

void wxPropertyValue::operator=(const int *val)
{
  m_modifiedFlag = true;
  m_type = wxPropertyValueIntegerPtr;
  m_value.integerPtr = (int*)val;
  m_clientData = NULL;
  m_next = NULL;
}

void wxPropertyValue::operator=(const long *val)
{
  m_modifiedFlag = true;
  m_type = wxPropertyValueLongPtr;
  m_value.longPtr = (long*)val;
  m_clientData = NULL;
  m_next = NULL;
}

void wxPropertyValue::operator=(const bool *val)
{
  m_modifiedFlag = true;
  m_type = wxPropertyValueboolPtr;
  m_value.boolPtr = (bool*)val;
  m_clientData = NULL;
  m_next = NULL;
}

void wxPropertyValue::operator=(const float *val)
{
  m_modifiedFlag = true;
  m_type = wxPropertyValueRealPtr;
  m_value.realPtr = (float*)val;
  m_clientData = NULL;
  m_next = NULL;
}

void wxPropertyValue::operator=(const double *val)
{
  m_modifiedFlag = true;
  m_type = wxPropertyValueDoublePtr;
  m_value.doublePtr = (double*)val;
  m_clientData = NULL;
  m_next = NULL;
}

long wxPropertyValue::IntegerValue(void) const
  {
    if (m_type == wxPropertyValueInteger)
      return m_value.integer;
    else if (m_type == wxPropertyValueReal)
      return (long)m_value.real;
    else if (m_type == wxPropertyValueIntegerPtr)
      return *m_value.integerPtr;
    else if (m_type == wxPropertyValueRealPtr)
      return (long)(*m_value.realPtr);
    else if (m_type == wxPropertyValueDoublePtr)
      return (long)(*m_value.doublePtr);
    else return 0;
  }

int *wxPropertyValue::IntegerValuePtr(void) const
{
  return m_value.integerPtr;
}

long *wxPropertyValue::LongValuePtr(void) const
{
  return m_value.longPtr;
}

float wxPropertyValue::RealValue(void) const
{
  if (m_type == wxPropertyValueReal)
    return m_value.real;
  else if (m_type == wxPropertyValueRealPtr)
    return *m_value.realPtr;
  else if (m_type == wxPropertyValueDoublePtr)
    return *m_value.doublePtr;
  else if (m_type == wxPropertyValueInteger)
    return (float)m_value.integer;
  else if (m_type == wxPropertyValueIntegerPtr)
    return (float)*(m_value.integerPtr);
  else return 0.0;
}

float* wxPropertyValue::RealValuePtr(void) const
{
  return m_value.realPtr;
}

double* wxPropertyValue::DoubleValuePtr(void) const
{
  return m_value.doublePtr;
}

bool wxPropertyValue::BoolValue(void) const
{
  if (m_type == wxPropertyValueReal)
    return (m_value.real != 0.0);
  if (m_type == wxPropertyValueRealPtr)
    return (*(m_value.realPtr) != 0.0f);
  else if (m_type == wxPropertyValueDoublePtr)
    return (*(m_value.doublePtr) != 0.0);
  else if (m_type == wxPropertyValueInteger)
    return (m_value.integer != 0);
  else if (m_type == wxPropertyValueIntegerPtr)
    return (*(m_value.integerPtr) != 0);
  else if (m_type == wxPropertyValuebool)
    return (m_value.integer != 0);
  else if (m_type == wxPropertyValueboolPtr)
    return (*(m_value.boolPtr) != 0);
  else return false;
}

bool *wxPropertyValue::BoolValuePtr(void) const
{
  return m_value.boolPtr;
}

wxChar *wxPropertyValue::StringValue(void) const
{
  if (m_type == wxPropertyValueString)
    return m_value.string;
  else if (m_type == wxPropertyValueStringPtr)
    return *(m_value.stringPtr);
  else
    return NULL;
}

wxChar **wxPropertyValue::StringValuePtr(void) const
{
  return m_value.stringPtr;
}

/*
 * A property (name plus value)
 */

IMPLEMENT_DYNAMIC_CLASS(wxProperty, wxObject)

wxProperty::wxProperty(void)
{
  m_propertyRole = wxEmptyString;
  m_propertyValidator = NULL;
  m_propertyWindow = NULL;
  m_enabled = true;
}

wxProperty::wxProperty(wxProperty& copyFrom)
    : wxObject()
{
  m_value = copyFrom.GetValue();
  m_name = copyFrom.GetName();
  m_propertyRole = copyFrom.GetRole();
  m_propertyValidator = copyFrom.GetValidator();
  m_enabled = copyFrom.IsEnabled();
  m_propertyWindow = NULL;
}

wxProperty::wxProperty(wxString nm, wxString role, wxPropertyValidator *ed)
  : m_name(nm),
    m_propertyRole(role)
{
  m_propertyValidator = ed;
  m_propertyWindow = NULL;
  m_enabled = true;
}

wxProperty::wxProperty(
  wxString nm,
  const wxPropertyValue& val,
  wxString role,
  wxPropertyValidator *ed)
  : m_value(val),
    m_name(nm),
    m_propertyRole(role)
{
  m_propertyValidator = ed;
  m_propertyWindow = NULL;
  m_enabled = true;
}

wxProperty::~wxProperty(void)
{
  if (m_propertyValidator)
    delete m_propertyValidator;
}

wxPropertyValue& wxProperty::GetValue(void) const
{
  return (wxPropertyValue&) m_value;
}

wxPropertyValidator *wxProperty::GetValidator(void) const
{
  return m_propertyValidator;
}

const wxString& wxProperty::GetName() const
{
  return (wxString&) m_name;
}

wxString& wxProperty::GetRole(void) const
{
  return (wxString&) m_propertyRole;
}

void wxProperty::SetValue(const wxPropertyValue& val)
{
  m_value = val;
}

void wxProperty::SetValidator(wxPropertyValidator *ed)
{
  m_propertyValidator = ed;
}

void wxProperty::SetRole(wxString& role)
{
  m_propertyRole = role;
}

void wxProperty::SetName(wxString& nm)
{
  m_name = nm;
}

void wxProperty::operator=(const wxPropertyValue& val)
{
  m_value = val;
}

/*
 * Base property view class
 */

IMPLEMENT_DYNAMIC_CLASS(wxPropertyView, wxEvtHandler)

wxPropertyView::wxPropertyView(long flags)
{
  m_buttonFlags = flags;
  m_propertySheet = NULL;
  m_currentValidator = NULL;
  m_currentProperty = NULL;
}

wxPropertyView::~wxPropertyView(void)
{
}

void wxPropertyView::AddRegistry(wxPropertyValidatorRegistry *registry)
{
  m_validatorRegistryList.push_back(registry);
}

wxPropertyValidator* wxPropertyView::FindPropertyValidator(
  wxProperty* pProperty)
{
  if (pProperty->GetValidator())
  {
    return pProperty->GetValidator();
  }

  for (auto& pPropertyValidatorRegistry : m_validatorRegistryList)
  {
    wxPropertyValidator* pPropertyValidator =
      pPropertyValidatorRegistry->GetValidator(pProperty->GetRole());
    if (pPropertyValidator)
    {
      return pPropertyValidator;
    }
  }

  return 0;
/*
  if (!wxDefaultPropertyValidator)
    wxDefaultPropertyValidator = new wxPropertyListValidator;
  return wxDefaultPropertyValidator;
*/
}

/*
 * Property sheet
 */

IMPLEMENT_DYNAMIC_CLASS(wxPropertySheet, wxObject)

wxPropertySheet::wxPropertySheet(const wxString& Name)
  : mProperties(),
    mName(Name)
{
}

wxPropertySheet::~wxPropertySheet(void)
{
  Clear();
}

void wxPropertySheet::UpdateAllViews( wxPropertyView *WXUNUSED(thisView) )
{
}

// Add a property
void wxPropertySheet::AddProperty(wxProperty *property)
{
  mProperties.insert(make_pair(property->GetName(), property));
}

// Get property by name
wxProperty *wxPropertySheet::GetProperty(const wxString& Name) const
{
  const auto iPropertyMap = mProperties.find(Name);
  if (iPropertyMap == mProperties.end())
  {
    return 0;
  }
  else
  {
    return iPropertyMap->second;
  }
}

bool wxPropertySheet::SetProperty(const wxString& Name, const wxPropertyValue& value)
{
  wxProperty* prop = GetProperty(Name);
  if (prop)
  {
    prop->SetValue(value);
    return true;
  }
  else
  {
    return false;
  }
}

void wxPropertySheet::RemoveProperty(const wxString& Name)
{
  auto iPropertyMap = mProperties.find(Name);
  if (iPropertyMap != mProperties.end())
  {
    delete iPropertyMap->second;
    mProperties.erase(iPropertyMap);
  }
}

bool wxPropertySheet::HasProperty(const wxString& Name) const
{
    return (GetProperty(Name)?true:false);
}

// Clear all properties
void wxPropertySheet::Clear(void)
{
  for (auto& NamePropertyPtrPair : mProperties)
  {
    delete NamePropertyPtrPair.second;
  }
  mProperties.clear();
}

// Sets/clears the modified flag for each property value
void wxPropertySheet::SetAllModified(bool flag)
{
  for (auto& NamePropertyPtrPair : mProperties)
  {
    wxProperty* pProperty = NamePropertyPtrPair.second;
    pProperty->GetValue().SetModified(flag);
  }
}

/*
 * Property validator registry
 *
 */

wxPropertyValidatorRegistry::wxPropertyValidatorRegistry()
  : mPropertyValidatorMap()
{
}

wxPropertyValidatorRegistry::~wxPropertyValidatorRegistry()
{
  ClearRegistry();
}

void wxPropertyValidatorRegistry::RegisterValidator(
  const wxString& TypeName,
  wxPropertyValidator* pValidator)
{
  mPropertyValidatorMap.insert(make_pair(TypeName, pValidator));
}

wxPropertyValidator *wxPropertyValidatorRegistry::GetValidator(
  const wxString& TypeName)
{
  auto iPropertyValidator = mPropertyValidatorMap.find(TypeName);
  if (iPropertyValidator != mPropertyValidatorMap.end())
  {
    return iPropertyValidator->second;
  }
  return 0;
}

void wxPropertyValidatorRegistry::ClearRegistry()
{
  for (auto& NamePropertyValuePtrPair : mPropertyValidatorMap)
  {
    wxPropertyValidator* pPropertyValidator = NamePropertyValuePtrPair.second;
    delete pPropertyValidator;
  }
  mPropertyValidatorMap.clear();
}

 /*
  * Property validator
  */


IMPLEMENT_ABSTRACT_CLASS(wxPropertyValidator, wxEvtHandler)

wxPropertyValidator::wxPropertyValidator(long flags)
{
  m_validatorFlags = flags;
  m_validatorProperty = NULL;
}

wxPropertyValidator::~wxPropertyValidator(void)
{}

bool wxPropertyValidator::StringToFloat (const wxString& s, float *number)
{
    double num;
    bool ok = StringToDouble (s, &num);
    *number = (float) num;
    return ok;
}

bool wxPropertyValidator::StringToDouble (const wxString& s, double *number)
{
    bool ok = true;
    wxChar *value_ptr;
    *number = wxStrtod (s, &value_ptr);
    if (value_ptr)
    {
        int len = wxStrlen (value_ptr);
        for (int i = 0; i < len; i++)
        {
            ok = (wxIsspace (value_ptr[i]) != 0);
            if (!ok) return false;
        }
    }
    return ok;
}

bool wxPropertyValidator::StringToInt (const wxString& s, int *number)
{
    long num;
    bool ok = StringToLong (s, &num);
    *number = (int) num;
    return ok;
}

bool wxPropertyValidator::StringToLong (const wxString& s, long *number)
{
    bool ok = true;
    wxChar *value_ptr;
    *number = wxStrtol (s, &value_ptr, 10);
    if (value_ptr)
    {
        int len = wxStrlen (value_ptr);
        for (int i = 0; i < len; i++)
        {
            ok = (wxIsspace (value_ptr[i]) != 0);
            if (!ok) return false;
        }
    }
    return ok;
}

wxChar *wxPropertyValidator::FloatToString (float number)
{
    static wxChar buf[20];
    wxSnprintf (buf, 20, wxT("%.6g"), number);
    return buf;
}

wxChar *wxPropertyValidator::DoubleToString (double number)
{
    static wxChar buf[20];
    wxSnprintf (buf, 20, wxT("%.6g"), number);
    return buf;
}

wxChar *wxPropertyValidator::IntToString (int number)
{
    static wxChar buf[20];

    wxSprintf (buf, wxT("%d"), number);
    return buf;
}

wxChar *wxPropertyValidator::LongToString (long number)
{
    static wxChar buf[20];

    wxSprintf (buf, wxT("%ld"), number);
    return buf;
}

#endif // wxUSE_PROPSHEET
