//*****************************************************************************
// The JAZZ++ Midi Sequencer
//
// Copyright (C) 1994-2000 Andreas Voss and Per Sigmond, all rights reserved.
// Modifications Copyright (C) 2004 Patrick Earl
// Modifications Copyright (C) 2008-2013 Peter J. Stieber
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//*****************************************************************************

#pragma once

#include <iostream>
#include <string>

// ------------------------------------------------------------------
// JZHarmonyBrowserChord
// ------------------------------------------------------------------

enum TEScaleType
{
  Major,
  Harmon,
  Melod,
  Ionb13,
  nScaleTypes
};


class JZHarmonyBrowserChord
{
    friend std::ostream& operator << (std::ostream& os, JZHarmonyBrowserChord const &a);
    friend std::istream& operator >> (std::istream& is, JZHarmonyBrowserChord &a);

  public:

    JZHarmonyBrowserChord()
    {
      keys = 0;
    }

    JZHarmonyBrowserChord(int k)
    {
      keys = k;
    }

    JZHarmonyBrowserChord(
      int a,
      int b,
      int c = -1,
      int d = -1,
      int e = -1,
      int f = -1,
      int g = -1,
      int h = -1,
      int i = -1,
      int k = -1,
      int l = -1);

    void Rotate(int semis);
    int Fit(int key);                // nearest key in chord
    int Count() const;                // # keys in chord
    int Iter(int key) const;        // next key in chord

    void Clear()
    {
      keys = 0;
    }

    int Keys() const
    {
      return keys;
    }

    bool Contains(int key) const        // key in chord?
    {
      return ((1L << ((key + 240) % 12)) & keys) != 0;
    }

    bool Contains(JZHarmonyBrowserChord const &o) const
    {
      return (keys & o.keys) == o.keys;
    }

    JZHarmonyBrowserChord & operator += (int key)
    {
      keys |= (1L << ((key + 240) % 12));
      return *this;
    }

    JZHarmonyBrowserChord & operator -= (int key)
    {
      keys &= ~(1L << ((key + 240) % 12));
      return *this;
    }

    JZHarmonyBrowserChord & operator -= (const JZHarmonyBrowserChord &o)
    {
      keys &= ~o.keys;
      return *this;
    }

    JZHarmonyBrowserChord operator - (const JZHarmonyBrowserChord &o) const
    {
      JZHarmonyBrowserChord ret(*this);
      ret.keys &= ~o.keys;
      return ret;
    }

    JZHarmonyBrowserChord & operator &= (const JZHarmonyBrowserChord & sc)
    {
      keys &= sc.keys;
      return *this;
    }

    JZHarmonyBrowserChord & operator |= (const JZHarmonyBrowserChord &sc)
    {
      keys |= sc.keys;
      return *this;
    }

    JZHarmonyBrowserChord & operator ^= (const JZHarmonyBrowserChord &sc)
    {
      keys ^= sc.keys;
      return *this;
    }

    JZHarmonyBrowserChord operator & (const JZHarmonyBrowserChord &sc) const
    {
      JZHarmonyBrowserChord ret(*this);
      ret &= sc;
      return ret;
    }

    JZHarmonyBrowserChord operator | (const JZHarmonyBrowserChord &sc) const
    {
      JZHarmonyBrowserChord ret(*this);
      ret |= sc;
      return ret;
    }

    JZHarmonyBrowserChord operator ^ (const JZHarmonyBrowserChord &sc) const
    {
      JZHarmonyBrowserChord ret(*this);
      ret ^= sc;
      return ret;
    }

    JZHarmonyBrowserChord operator + (int key)
    {
      JZHarmonyBrowserChord ret(*this);
      ret += key;
      return ret;
    }

    JZHarmonyBrowserChord operator - (int key)
    {
      JZHarmonyBrowserChord ret(*this);
      ret -= key;
      return ret;
    }

    bool operator == (const JZHarmonyBrowserChord &o) const
    {
      return (keys == o.keys);
    }

    bool operator != (const JZHarmonyBrowserChord &o) const
    {
      return (keys != o.keys);
    }

    void flat(int key)
    {
      *this -= key;
      *this += key-1;
    }

    void sharp(int key)
    {
      *this -= key;
      *this += key+1;
    }

    void CreateName(std::string& CordName, int key, int flat = 0);

    static const std::string ScaleName(int key, int flat = 0)
    {
      return mScaleNames[flat != 0][(key + 240) % 12];
    }

  protected:
    int keys;

  private:

    static const std::string mScaleNames[2][12];
};

inline std::ostream& operator << (std::ostream& os, JZHarmonyBrowserChord const &a)
{
  os << a.keys << std::endl;
  return os;
}

inline std::istream& operator >> (std::istream& is, JZHarmonyBrowserChord &a)
{
  is >> a.keys;
  return is;
}


// scales, msb = highest note
const JZHarmonyBrowserChord major_scale       (0xab5L);
const JZHarmonyBrowserChord harmonic_scale         (0x9adL);
const JZHarmonyBrowserChord melodic_scale         (0xaadL);
const JZHarmonyBrowserChord ionb13_scale         (0x9b5L);

const JZHarmonyBrowserChord altered_scale        (0, 1, 3, 4, 6, 8, 10);
const JZHarmonyBrowserChord dimin_scale       (0xb6dL);
const JZHarmonyBrowserChord chromatic_scale         (0xfffL);

// chords, based to C
const JZHarmonyBrowserChord Cj7        (0,4,7,11);
const JZHarmonyBrowserChord Cj7b5        (0,4,6,11);
const JZHarmonyBrowserChord Cj7s5        (0,4,8,11);

const JZHarmonyBrowserChord C7        (0,4,7,10);
const JZHarmonyBrowserChord C7b5        (0,4,6,10);
const JZHarmonyBrowserChord C7s5        (0,4,8,10);

const JZHarmonyBrowserChord Cm7        (0,3,7,10);
const JZHarmonyBrowserChord Cm7b5        (0,3,6,10);
const JZHarmonyBrowserChord Cm7s5        (0,3,8,10);

const JZHarmonyBrowserChord Cmj7        (0,3,7,11);
const JZHarmonyBrowserChord Cmj7b5        (0,3,6,11);
const JZHarmonyBrowserChord Cmj7s5        (0,3,8,11);

const JZHarmonyBrowserChord C0        (0,3,6,9);

// ------------------------------------------------------------------
// JZHarmonyBrowserContext
// ------------------------------------------------------------------

#define NAME_TABLE 0

class JZHarmonyBrowserContext
{
    friend class JZHarmonyBrowserContextIterator;
    friend std::ostream& operator << (std::ostream& os, JZHarmonyBrowserContext const &a);
    friend std::istream& operator >> (std::istream& is, JZHarmonyBrowserContext &a);

  public:

    JZHarmonyBrowserContext(int sn, int cn = 0, TEScaleType st = Major);

    JZHarmonyBrowserContext();

    JZHarmonyBrowserChord *PScale()
    {
      return &scale;
    }

    JZHarmonyBrowserChord Scale() const
    {
      return scale;
    }

    JZHarmonyBrowserChord *PChord()
    {
      return &chord;
    }

    JZHarmonyBrowserChord Chord() const
    {
      return chord;
    }

    int ScaleKey() const
    {
      return scale_nr;
    }

    int ChordKey() const
    {
      return chord_key;
    }

    int ChordNr() const
    {
      return chord_nr;
    }

    int ScaleNr() const
    {
      return scale_nr;
    }

    TEScaleType ScaleType() const
    {
      return scale_type;
    }

    int SeqNr() const
    {
      return seq_nr;
    }

    void SetSeqNr(int n = 0)
    {
      seq_nr = n;
    }

    // For example "Dm75-"
    std::string GetChordName() const;

    // For example "IV"
    const char* ChordNrName() const;

    const char* ContextName() const               // "mixo#11"
    {
      return context_names[scale_type][chord_nr];
    }

    // For example "C#"
    const std::string& GetScaleName() const;

    const char* ScaleTypeName() const;            // "major"

    int operator == (const JZHarmonyBrowserContext& Rhs) const
    {
      return
        scale_type == Rhs.scale_type &&
        scale_nr   == Rhs.scale_nr &&
        chord_nr   == Rhs.chord_nr;
    }

    int operator != (const JZHarmonyBrowserContext& Rhs) const
    {
      return !operator == (Rhs);
    }

  private:

    void Initialize();

    JZHarmonyBrowserChord MakeScale() const;

    JZHarmonyBrowserChord MakeChord() const;

    int MakeChordKey() const;

    TEScaleType scale_type;

    int scale_nr;

    int chord_nr;

    int seq_nr;

    JZHarmonyBrowserChord chord;

    JZHarmonyBrowserChord scale;

    int chord_key;

#if NAME_TABLE
    static const char *const chord_names[nScaleTypes][7];
#endif

    static const char* const chord_nr_names[7];
    static const char* const scale_type_names[nScaleTypes];
    static const int         flat_keys[12];
    static const char* const context_names[nScaleTypes][7];
};


// ------------------------------------------------------------------
// JZHarmonyBrowserContextIterator
// ------------------------------------------------------------------

class JZHarmonyBrowserMatch
{
  public:

    virtual ~JZHarmonyBrowserMatch()
    {
    }

    virtual bool operator()(const JZHarmonyBrowserContext &)
    {
      return true;
    }
};

class JZHarmonyBrowserMatchContains : public JZHarmonyBrowserMatch
{
  public:

    JZHarmonyBrowserMatchContains(JZHarmonyBrowserChord c)
      : chord(c)
    {
    }

    virtual bool operator()(const JZHarmonyBrowserContext &iter);

  private:

    JZHarmonyBrowserChord chord;
};


class JZHarmonyBrowserContextIterator
{
  public:

    JZHarmonyBrowserContextIterator();

    JZHarmonyBrowserContextIterator(JZHarmonyBrowserMatch &);

    void SetSequence(JZHarmonyBrowserContext *s[], int n)
    {
      seq = s;
      n_seq = n;
    }

    void SetScaleType(TEScaleType st)
    {
      scale_type = st;
    }

    bool operator()();

    const JZHarmonyBrowserContext *operator->() const
    {
      return &context;
    }

    const JZHarmonyBrowserContext& Context() const
    {
      return context;
    }

  private:

    JZHarmonyBrowserContext context;

    JZHarmonyBrowserMatch& match;

    JZHarmonyBrowserMatch def_match;

    JZHarmonyBrowserContext** seq;

    int i_seq, n_seq;

    TEScaleType scale_type;
};
