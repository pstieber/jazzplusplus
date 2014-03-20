/////////////////////////////////////////////////////////////////////////////
// Name:        prop.h
// Purpose:     Property sheet classes
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// RCS-ID:      $Id: prop.h 35650 2005-09-23 12:56:45Z MR $
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PROP_H_
#define _WX_PROP_H_

#include "setup.h"

#if wxUSE_PROPSHEET

#include "wx/defs.h"
#include "wx/string.h"
#include "wx/hash.h"
#include "wx/dialog.h"
#include "wx/frame.h"
#include "wx/button.h"
#include "wx/listbox.h"
#include "wx/textctrl.h"
#include "wx/gdicmn.h"
#include "wx/layout.h"
#include "wx/sizer.h"

#include <map>
#include <list>

class wxWindow;
class wxProperty;
class wxPropertyValue;
class wxPropertySheet;
class wxPropertyView;
class wxPropertyValidator;
class wxPropertyValidatorRegistry;

#define wxPROPERTY_VERSION 2.0

// A storable sheet of values
class wxPropertySheet: public wxObject
{
  public:

    wxPropertySheet(const wxString& Name = wxT(""));

    ~wxPropertySheet();

    // Set the name of the sheet
    inline
    virtual void SetName(const wxString& Name)
    {
      mName = Name;
    }

    inline
    virtual wxString GetName() const
    {
      return mName;
    }

    // Does this sheet contain a property with this name
    virtual bool HasProperty(const wxString& Name) const;

    // Set property name to value
    virtual bool SetProperty(const wxString& Name, const wxPropertyValue& value);

    // Remove property from sheet by name, deleting it
    virtual void RemoveProperty(const wxString& Name);

    // Get the name of the sheet
    // Add a property
    virtual void AddProperty(wxProperty *property);

    // Get property by name
    virtual wxProperty *GetProperty(const wxString& Name) const;

    // Clear all properties
    virtual void Clear();

    virtual void UpdateAllViews(wxPropertyView *thisView = NULL);

    virtual const std::map<wxString, wxProperty*>& GetProperties() const;

    // Sets/clears the modified flag for each property value
    virtual void SetAllModified(bool flag = true);

  protected:

    wxObject* m_viewedObject;
    std::map<wxString, wxProperty*> mProperties;
    wxPropertyView* m_propertyView;
    wxString mName;

  private:

    DECLARE_DYNAMIC_CLASS(wxPropertySheet)
};


// Base class for property sheet views. There are currently two directly derived
// classes: wxPropertyListView, and wxPropertyFormView.
class wxPropertyView: public wxEvtHandler
{
  public:

    wxPropertyView(long flags = 0);
    ~wxPropertyView();

    // Associates and shows the view
    virtual void ShowView(wxPropertySheet *WXUNUSED(propertySheet), wxWindow *WXUNUSED(panel))
    {
    }

    // Update this view of the viewed object, called e.g. by
    // the object itself.
    virtual bool OnUpdateView()
    {
      return false;
    }

    // Override this to do something as soon as the property changed,
    // if the view and validators support it.
    virtual void OnPropertyChanged(wxProperty *WXUNUSED(property))
    {
    }

    virtual void AddRegistry(wxPropertyValidatorRegistry *registry);
    inline
    virtual const std::list<wxPropertyValidatorRegistry*>& GetRegistryList() const
    {
      return m_validatorRegistryList;
    }

    virtual wxPropertyValidator* FindPropertyValidator(wxProperty* pProperty);

    inline
    virtual void SetPropertySheet(wxPropertySheet* sheet)
    {
      m_propertySheet = sheet;
    }

    inline
    virtual wxPropertySheet *GetPropertySheet() const
    {
      return m_propertySheet;
    }

    inline virtual bool OnClose()
    {
      return false;
    }

    inline
    long GetFlags(void)
    {
      return m_buttonFlags;
    }

  protected:

    long                  m_buttonFlags;
    wxPropertySheet*      m_propertySheet;
    wxProperty*           m_currentProperty;
    std::list<wxPropertyValidatorRegistry*> m_validatorRegistryList;
    wxPropertyValidator*  m_currentValidator;

  private:

    DECLARE_DYNAMIC_CLASS(wxPropertyView)
};


class wxPropertyValidator: public wxEvtHandler
{
  public:

    wxPropertyValidator(long flags = 0);
    ~wxPropertyValidator();

    inline long GetFlags() const
    {
      return m_validatorFlags;
    }
    inline void SetValidatorProperty(wxProperty *prop)
    {
      m_validatorProperty = prop;
    }
    inline wxProperty *GetValidatorProperty(void) const
    {
      return m_validatorProperty;
    }

    virtual bool StringToFloat (const wxString& s, float *number);
    virtual bool StringToDouble (const wxString& s, double *number);
    virtual bool StringToInt (const wxString& s, int *number);
    virtual bool StringToLong (const wxString& s, long *number);
    virtual wxChar *FloatToString (float number);
    virtual wxChar *DoubleToString (double number);
    virtual wxChar *IntToString (int number);
    virtual wxChar *LongToString (long number);

  protected:

    long          m_validatorFlags;
    wxProperty*   m_validatorProperty;

  private:

    DECLARE_DYNAMIC_CLASS(wxPropertyValidator)
};


// extern wxPropertyValidator *wxDefaultPropertyValidator;

class wxPropertyValidatorRegistry
{
  public:

    wxPropertyValidatorRegistry();

    virtual ~wxPropertyValidatorRegistry();

    virtual void RegisterValidator(
      const wxString& RoleName,
      wxPropertyValidator* pValidator);

    virtual wxPropertyValidator* GetValidator(const wxString& RoleName);

    void ClearRegistry();

  private:

    std::map<wxString, wxPropertyValidator*> mPropertyValidatorMap;
};

/*
 * Property value class
 */

typedef enum
{
  wxPropertyValueNull,
  wxPropertyValueInteger,
  wxPropertyValueReal,
  wxPropertyValuebool,
  wxPropertyValueString,
  wxPropertyValueList,
  wxPropertyValueIntegerPtr,
  wxPropertyValueLongPtr,
  wxPropertyValueRealPtr,
  wxPropertyValueDoublePtr,
  wxPropertyValueboolPtr,
  wxPropertyValueStringPtr
} wxPropertyValueType;

class wxPropertyValue: public wxObject
{
  DECLARE_DYNAMIC_CLASS(wxPropertyValue)

  wxPropertyValue(void);                       // Unknown type
  wxPropertyValue(const wxPropertyValue& copyFrom);  // Copy constructor
  wxPropertyValue(const wxChar *val);
  wxPropertyValue(const wxString& val);
//  wxPropertyValue(int val);
  wxPropertyValue(long val);
  wxPropertyValue(bool val);
  wxPropertyValue(float val);
  wxPropertyValue(double the_real);
  wxPropertyValue(wxList *val);
  wxPropertyValue(wxStringList *val);
  // Pointer versions
  wxPropertyValue(wxChar **val);
  wxPropertyValue(int *val);
  wxPropertyValue(long *val);
  wxPropertyValue(bool *val);
  wxPropertyValue(float *val);
  wxPropertyValue(double *val);

  ~wxPropertyValue(void);

  virtual inline wxPropertyValueType Type(void) const
  {
    return m_type;
  }
  virtual inline void SetType(wxPropertyValueType typ)
  {
    m_type = typ;
  }
  virtual long IntegerValue(void) const;
  virtual float RealValue(void) const;
  virtual bool BoolValue(void) const;
  virtual wxChar* StringValue(void) const;
  virtual int* IntegerValuePtr(void) const;
  virtual long* LongValuePtr(void) const;
  virtual float* RealValuePtr(void) const;
  virtual double* DoubleValuePtr(void) const;
  virtual bool* BoolValuePtr(void) const;
  virtual wxChar** StringValuePtr(void) const;

  // Get nth arg of clause (starting from 1)
  virtual wxPropertyValue *Arg(wxPropertyValueType type, int arg) const;

  // Return nth argument of a list expression (starting from zero)
  virtual wxPropertyValue *Nth(int arg) const;
  // Returns the number of elements in a list expression
  virtual int Number(void) const;

  virtual wxPropertyValue *NewCopy(void) const;
  virtual void Copy(wxPropertyValue& copyFrom);

  virtual void WritePropertyClause(wxString &stream);  // Write this expression as a top-level clause
  virtual void WritePropertyType(wxString &stream);    // Write as any other subexpression

  // Append an expression to a list
  virtual void Append(wxPropertyValue *expr);
  // Insert at beginning of list
  virtual void Insert(wxPropertyValue *expr);

  // Get first expr in list
  virtual inline wxPropertyValue *GetFirst(void) const
    { return ((m_type == wxPropertyValueList) ? m_value.first : (wxPropertyValue*)NULL); }

  // Get next expr if this is a node in a list
  virtual inline wxPropertyValue *GetNext(void) const
    { return m_next; }

  // Get last expr in list
  virtual inline wxPropertyValue *GetLast(void) const
  { return ((m_type == wxPropertyValueList) ? m_last : (wxPropertyValue*)NULL); }

  // Delete this node from the list
  virtual void Delete(wxPropertyValue *node);

  // Clear list
  virtual void ClearList(void);

  virtual inline void SetClientData(wxObject *data)
  {
    m_clientData = data;
  }
  virtual inline wxObject *GetClientData(void)
  {
    return m_clientData;
  }

  virtual wxString GetStringRepresentation(void);

  inline void SetModified(bool flag = true)
  {
    m_modifiedFlag = flag;
  }
  inline bool GetModified(void)
  {
    return m_modifiedFlag;
  }

  // Operators
  void operator=(const wxPropertyValue& val);
//  void operator=(const char *val);
  void operator=(const wxString& val);
  void operator=(const long val);
  void operator=(const bool val);
  void operator=(const float val);
  void operator=(const wxChar **val);
  void operator=(const int *val);
  void operator=(const long *val);
  void operator=(const bool *val);
  void operator=(const float *val);
  void operator=(const double *val);

 public:
  wxObject*             m_clientData;
  wxPropertyValueType   m_type;
  bool                  m_modifiedFlag;

  union
  {
    long integer; // Also doubles as bool
    wxChar* string;
    float real;
    int* integerPtr;
    long* longPtr;
    bool* boolPtr;
    wxChar **stringPtr;
    float* realPtr;
    double* doublePtr;
    wxPropertyValue *first;  // If is a list expr, points to the first node
    } m_value;

  wxPropertyValue*      m_next;     // If this is a node in a list, points to the next node
  wxPropertyValue*      m_last;     // If is a list expr, points to the last node

};

/*
 * Property class: contains a name and a value.
 */

class wxProperty: public wxObject
{
  DECLARE_DYNAMIC_CLASS(wxProperty)
  protected:
    bool                  m_enabled;
  public:
    wxPropertyValue       m_value;
    wxString              m_name;
    wxString              m_propertyRole;
    wxPropertyValidator*  m_propertyValidator;
    wxWindow*             m_propertyWindow; // Usually a panel item, if anything

    wxProperty(void);
    wxProperty(wxProperty& copyFrom);
    wxProperty(wxString Name, wxString role, wxPropertyValidator *ed = NULL);
    wxProperty(wxString Name, const wxPropertyValue& val, wxString role, wxPropertyValidator *ed = NULL);
    virtual ~wxProperty(void);

    virtual wxPropertyValue& GetValue(void) const;
    virtual wxPropertyValidator *GetValidator(void) const;
    virtual const wxString& GetName() const;
    virtual wxString& GetRole() const;
    virtual void SetValue(const wxPropertyValue& val);
    virtual void SetValidator(wxPropertyValidator *v);
    virtual void SetName(wxString& nm);
    virtual void SetRole(wxString& role);
    void operator=(const wxPropertyValue& val);
    virtual inline void SetWindow(wxWindow *win)
    {
      m_propertyWindow = win;
    }
    virtual inline wxWindow *GetWindow(void) const
    {
      return m_propertyWindow;
    }

    inline void Enable(bool en)
    {
      m_enabled = en;
    }
    inline bool IsEnabled(void) const
    {
      return m_enabled;
    }
};

inline
const std::map<wxString, wxProperty*>& wxPropertySheet::GetProperties() const
{
  return mProperties;
}

#endif // wxUSE_PROPSHEET

#endif // _WX_PROP_H_
