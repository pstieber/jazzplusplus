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

#include "HarmonyP.h"

#include <assert.h>
#include <string.h>

#include <iostream>

using namespace std;

// ========================================================================
// JZHarmonyBrowserChord
// ========================================================================

const string JZHarmonyBrowserChord::mScaleNames[2][12] =
{
  { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" },
  { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" }
};


JZHarmonyBrowserChord::JZHarmonyBrowserChord(int a, int b, int c, int d, int e, int f, int g, int h, int i, int k, int l)
{
  keys = 0L;
  if (a >= 0) operator += (a);
  if (b >= 0) operator += (b);
  if (c >= 0) operator += (c);
  if (d >= 0) operator += (d);
  if (e >= 0) operator += (e);
  if (f >= 0) operator += (f);
  if (g >= 0) operator += (g);
  if (h >= 0) operator += (h);
  if (i >= 0) operator += (i);
  if (k >= 0) operator += (k);
  if (l >= 0) operator += (l);
}

void JZHarmonyBrowserChord::CreateName(string& ChordName, int key, int flat)
{
  JZHarmonyBrowserChord c = *this;
  ChordName = ScaleName(key, flat);

  // Transpose to C.
  c.Rotate(-key);

  // Handle special cases.
  if (c == C0)
  {
    ChordName.append("o");
    return;
  }

  bool seven = false;
  bool sharp9 = false;
  bool nat11  = false;
  bool sharp11 = false;
  bool nat13 = false;
  bool flat13 = false;

  // minor?
  if (c.Contains(3))
  {
    if (!c.Contains(4))
    {
      ChordName.append("m");
    }
    else
    {
      sharp9 = true;
    }
  }

  // 7
  if (c.Contains(11))
  {
    ChordName.append("j7");
    seven = true;
  }
  else if (c.Contains(10))
  {
    ChordName.append("7");
    seven = true;
  }

  // 4
  if (c.Contains(5))
  {
    if (!c.Contains(4))
    {
      ChordName.append("sus4");
    }
    else
    {
      nat11 = true;
    }
  }

  // 5
  if (c.Contains(7))
  {
    if (c.Contains(6))
    {
      sharp11 = true;
    }
    if (c.Contains(8))
    {
      flat13 = true;
    }
  }
  else
  {
    if (c.Contains(6))
    {
      ChordName.append("5-");
    }
    if (c.Contains(8))
    {
      ChordName.append("5+");
    }
  }

  // 6
  if (c.Contains(9))
  {
    if (!seven)
    {
      ChordName.append("6");
    }
    else
    {
      nat13 = true;
    }
  }

  // 9
  if (c.Contains(1))
  {
    ChordName.append("9-");
  }
  if (c.Contains(2))
  {
    ChordName.append("5");
  }

  if (sharp9)
  {
    ChordName.append("9+");
  }

  // 11
  if (nat11)
  {
    ChordName.append("11 ");
  }
  if (sharp11)
  {
    ChordName.append("11+");
  }

  // 13
  if (flat13)
  {
    ChordName.append("13-");
  }
  if (nat13)
  {
    ChordName.append("13");
  }
}


int JZHarmonyBrowserChord::Count() const
{
  int i, n = 0;
  for (i = 0; i < 12; i++)
    if (keys & (1 << i))
      n++;
  return n;
}


int JZHarmonyBrowserChord::Iter(int key) const
{
  assert(keys);
  key++;
  while (!Contains(key))
    key++;
  return key;
}


void JZHarmonyBrowserChord::Rotate(int semis)
{
  if (semis > 0)
  {
    while (semis--)
    {
      if (keys & 0x800)
        keys = ((keys & ~0x800) << 1) + 1;
      else keys <<= 1;
    }
  }
  else if (semis < 0)
  {
    while (semis++)
    {
      if (keys & 1)
        keys = (keys >> 1) | 0x800;
      else keys >>= 1;
    }
  }
}


int JZHarmonyBrowserChord::Fit(int key)
{
  assert(keys);
  for (int i = 1; !Contains(key) && i < 12; i++)
  {
    if (i & 1)
      key -= i;
    else
      key += i;
  }
  return key;
}

// ========================================================================
// JZHarmonyBrowserContext
// ========================================================================

#if NAME_TABLE
const char *const JZHarmonyBrowserContext::chord_names[nScaleTypes][7] = {
// major
  { "j7", "m7", "m7", "j7", "7", "m7", "m75-"},
// harm
  { "mj7", "m75-", "j75+", "m7", "7", "j7", "0"},
// melod
  { "mj7", "m7", "j75+", "7", "7", "m75-", "m75-"},
// ionic b13
  { "maj7", "m75-", "7", "mj7", "7", "j75+", "0"}

};
#endif

const int JZHarmonyBrowserContext::flat_keys[12] =
//  c     d     e  f     g     a     b
  { 1, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0 };

const char *const JZHarmonyBrowserContext::chord_nr_names[7] =
  { "I", "II", "III", "IV", "V", "VI", "VII"};

const char *const JZHarmonyBrowserContext::context_names[nScaleTypes][7] =
{
  {
    "ion",
    "dor",
    "phry",
    "lyd",
    "mixo",
    "aeol",
    "locr"
  },
  {
    "harm",
    "locr 13",
    "ion #5",
    "dor #11",
    "HM5",
    "lyd #9",
    "har dim"
  },
  {
    "melod",
    "dor b9",
    "lyd #5",
    "mixo #11",
    "mixo b13",
    "locr 9",
    "superlocr"
  },
  {
    "ion b13",
    "dor b5",
    "harm alt",
    "melod #11",
    "mixo b9",
    "lyd #9#5",
    "locr dim"
  },
};

const char* const JZHarmonyBrowserContext::scale_type_names[nScaleTypes] =
{
  "J",
  "HM",
  "MM",
  "HJ"
};


JZHarmonyBrowserContext::JZHarmonyBrowserContext(int sn, int cn, TEScaleType st)
{
  scale_type = st;
  scale_nr   = sn % 12;
  chord_nr   = cn % 7;
  seq_nr     = 0;
  Initialize();
}

JZHarmonyBrowserContext::JZHarmonyBrowserContext()
{
  scale_type = Major;
  scale_nr   = 0;
  chord_nr   = 0;
  seq_nr     = 0;
  Initialize();
}

string JZHarmonyBrowserContext::GetChordName() const
{
#if NAME_TABLE
  // Use the table of chord names (fast).
  int chord_key = ChordKey();
  string ChordName = JZHarmonyBrowserChord::ScaleName(chord_key, flat_keys[scale_nr]);
  ChordName.append(chord_names[scale_type][chord_nr]);
  return ChordName;
#else
  // Compute the chord name (slow, but flexible).
  int chord_key = ChordKey();
  JZHarmonyBrowserChord chord = Chord();
  string ChordName;
  chord.CreateName(ChordName, chord_key, flat_keys[chord_key]);
  return ChordName;
#endif
}

const char * JZHarmonyBrowserContext::ChordNrName() const
{
  return chord_nr_names[chord_nr];        // "IV"
}

const string& JZHarmonyBrowserContext::GetScaleName() const
{
  static string ScaleName;
  //strcpy(buf, scale_names[flat_keys[scale_nr]][scale_nr]);
  ScaleName = JZHarmonyBrowserChord::ScaleName(scale_nr, flat_keys[scale_nr]);
#if 0
  strcat(buf, "/");
  strcat(buf, scale_type_names[scale_type]);
#endif
  return ScaleName;
}

const char * JZHarmonyBrowserContext::ScaleTypeName() const
{
  return scale_type_names[scale_type];
}


JZHarmonyBrowserChord JZHarmonyBrowserContext::MakeScale() const
{
  JZHarmonyBrowserChord scale;
  switch (scale_type)
  {
    case Major:
      scale = major_scale;
      break;
    case Harmon:
      scale = harmonic_scale;
      break;
    case Melod:
      scale = melodic_scale;
      break;
    case Ionb13:
      scale = ionb13_scale;
      break;
    default:
      scale = 0;
      break;
  }
  scale.Rotate(scale_nr);
  return scale;
}


JZHarmonyBrowserChord JZHarmonyBrowserContext::MakeChord() const
{
  int i, j;
  JZHarmonyBrowserChord chord;
  int key = scale_nr;
  for (i = 0; i < chord_nr; i++)
  {
    key = scale.Iter(key);
  }
  chord += key;
  for (i = 1; i < 4; i++)
  {
    for (j = 0; j < 2; j++)
    {
      key = scale.Iter(key);
    }
    chord += key;
  }
  return chord;
}


int JZHarmonyBrowserContext::MakeChordKey() const
{
  int key = scale_nr;
  for (int i = 0; i < chord_nr; i++)
  {
    key = scale.Iter(key);
  }
  return key % 12;
}


void JZHarmonyBrowserContext::Initialize()
{
  scale      = MakeScale();
  chord      = MakeChord();
  chord_key  = MakeChordKey();
}

ostream & operator << (ostream &os, JZHarmonyBrowserContext const &a)
{
  os << (int) a.scale_type << ' ';
  os << a.scale_nr << ' ';
  os << a.chord_nr << ' ';
  os << a.seq_nr << ' ';
  os << a.chord_key << ' ';
  os << a.chord << a.scale;
  return os;
}

istream & operator >> (istream &is, JZHarmonyBrowserContext &a)
{
  int sc;
  is >> sc >> a.scale_nr >> a.chord_nr >> a.seq_nr >> a.chord_key;
  is >> a.chord >> a.scale;
  a.scale_type = (TEScaleType)sc;
  return is;
}

// ========================================================================
// JZHarmonyBrowserContextIterator
// ========================================================================

bool JZHarmonyBrowserMatchContains::operator()(const JZHarmonyBrowserContext &context)
{
  return context.Chord().Contains(chord);
}

JZHarmonyBrowserContextIterator::JZHarmonyBrowserContextIterator()
  : match(def_match)
{
  context.scale_type = (TEScaleType)0;
  context.scale_nr   = 0;
  context.chord_nr   = -1;
  i_seq = n_seq = 0;
  scale_type = nScaleTypes;
}

JZHarmonyBrowserContextIterator::JZHarmonyBrowserContextIterator(JZHarmonyBrowserMatch &m)
  : match(m)
{
  context.scale_type = (TEScaleType)0;
  context.scale_nr   = 0;
  context.chord_nr   = -1;
  i_seq = n_seq = 0;
}

// PAT - Changed this to bool since it seemed more consistent with what it was
// returning.  There was previously a conflict with the prototype in
// harmonyp.h due to the commenting of #define bool int.
bool JZHarmonyBrowserContextIterator::operator()()
{
  while (!i_seq && context.scale_type < nScaleTypes)
  {
    if (scale_type != nScaleTypes && context.scale_type != scale_type)
    {
      context.scale_type = (TEScaleType)((int)context.scale_type + 1);
      continue;
    }

    while (context.scale_nr < 12)
    {
      while (context.chord_nr < 6)
      {
        ++ context.chord_nr;
        context.Initialize();
        if (match(context))
          return true;
      }
      context.chord_nr = -1;
      context.scale_nr ++;
    }
    context.scale_nr = 0;
    context.scale_type = (TEScaleType)((int)context.scale_type + 1);
  }

  while (i_seq < n_seq)
  {
    int i = i_seq++;
    if (match(*seq[i]))
    {
      context = *seq[i];
      return true;
    }
  }
  return false;
}

// ---------------------------------------------------------------

#if 0
int main()
{
  JZHarmonyBrowserContextIterator iter;
  while (iter())
  {
    cout << "ScaleName  : " << iter->ScaleName() << endl
         << "ChordNrName: " << iter->ChordNrName() << endl
         << "ChordName  : " << iter->ChordName() << endl
         << endl;
  }
  return 0;
}
#endif

#if 0
void db(const JZHarmonyBrowserChord c)
{
  for (int i = 0; i < 12; i++)
    if (c.Contains(i))
      cout << i << ' ';
  cout << endl;
}

int main()
{
  db(major_scale);
  db(harmonic_scale);
  db(melodic_scale);
  db(dimin_scale);
}
#endif



// --------------------------------------------------------------


#if 0
#include <ctype.h>
struct scdef
{
  const char *name;
  const char *def;
};

scdef scales[] =  {
  { "***** major scales *****", ""},
  { "major penta",         "1 9 3 5 13" },
  { "ionic",                "1 9 3 11 5 13 j7" },
  { "lydic",                "1 9 3 11+ 5 13 j7"},
  { "har III (ion 5+)",        "1 9 3 11 5+ 13 j7"},
  { "har VI (lyd 9+)",        "1 9+ 3 11+ 5 13 j7"},
  { "mel III (lyd 5+)",        "1 9 3 11+ 5+ 13 j7"},
  { "augmented",        "1 9- 3 11 5+ 13"},
  { "ionic 13-",        "1 9 3 11 5 13- j7"},
  { "***** minor scales *****", ""},
  { "minor penta",         "1 3- 11 5 7"},
  { "aeolic",                "1 9 3- 11 5 13- 7"},
  { "doric",                "1 9 3- 11 5 13 7"},
  { "mel II (doric 9-)","1 9- 3- 11 5 13 7"},
  { "phrygic",                "1 9- 3- 11 5 13- 7"},
  { "jap. penta",        "1 9- 11 5 7"},
  { "har IV (dor 11+)",        "1 9 3- 11+ 5 13 7"},
  { "harmonic minor",        "1 9 3- 11 5 13- j7"},
  { "melodic minor",        "1 9 3- 11 5 13 j7"},
  { "gipsy",                "1 9 3- 11+ 5 13- j7" },
  { "melodic 11+",        "1 9 3- 11+ 5 13 j7" },
  { "***** dominant scales *****", "" },
  { "major penta",        "1 9 3 5 13" },
  { "ind. penta",         "1 3 11 5 7" },
  { "mixolyd",                "1 9 3 11 5 13 7" },
  { "har V (har dominant)",        "1 9- 3 11 5 13- 7" },
  { "mel IV (mixo 11+)","1 9 3 11+ 5 13 7" },
  { "mixo 11+9-",        "1 9- 3 11+ 5 13 7" },
  { "mel V (mixo 13-)",        "1 9 3 11 5 13- 7" },
  { "mixo 9-",                "1 9- 3 11 5 13 7" },
  { "full 1",                "1 9 3 5- 13- 7" },
  { "full 2",                "1 9 3 11+ 5+ 7" },
  { "har alt",                "1 9- 9+ 3 5 13- 7" },
  { "mel VII (altered)","1 9- 9+ 3 11+ 13- 7" },
  { "semi/full",        "1 9- 9+ 3 11+ 5 13 7" },
  { "***** semi dimin *****", ""},
  { "locrian",                "1 9- 3- 11 5- 13- 7" },
  { "mel VI (locrian 9)", "1 9 3- 11 5- 13- 7" },
  { "har II (locrian 13)", "1 9- 3- 11 5- 13 7" },
  { "doric 5-",         "1 9 3- 11 5- 13 7" },
  { "***** dimin *****", ""},
  { "har VII (har dim)","1 9- 3- 11- 5- 13- 7-"},
  { "full/semi",        "1 9 3- 11 5- 13 7- 15-"},
  { "locrian dim",        "1 9- 3- 11 5- 13- 7-" },
  { "***** blues scales *****", "" },
  { "minor penta 5-",        "1 3- 11 5- 5 7" },
  { "blues scale",        "1 3- 3 11 5- 5 7"},
  { 0, 0 }
};


scdef chords[] =  {
  { "j7",                "1 3 5 j7"},
  { "m7",                "1 3- 5 7" },
  { "7",                "1 3 5 7" },
  { "m75-",                "1 3- 5- 7" },
  { "mj7",                "1 3- 5 j7" },
  { "j75+",                "1 3 5+ j7" },
  { "0",                "1 3- 5- 6" },
  { "sus4",                "1 4 5" },
  { "7sus4",                "1 4 5 7" },
  { "j7sus4",                "1 4 5 j7" },
  { "alt (79+13-)",        "1 3 9+ 13- 7" },
  { "75-",                "1 3 5- 7" },
};

int scansc(const char *s)
{
  JZHarmonyBrowserChord res(0);
  while (*s)
  {
    while (isspace(*s))
      s++;
    int k = 0;
    int j7 = 0;
    if (*s == 'j')
    {
      j7 = 1;
      s++;
    }
    while (isdigit(*s))
      k = k * 10 + *s++ - '0';
    int kk = 0;
    for (int i = 1; i < k; i++)
      kk = major_scale.Iter(kk);
    if (k == 7)
      kk--;
    if (j7)
      kk++;
    if (*s == '+')
    {
      ++ kk;
      ++ s;
    }
    else if (*s == '-')
    {
      -- kk;
      ++ s;
    }
    res += kk;
  }
  return res.Keys();
}

void gensc(scdef *scd)
{
  while (scd->name)
  {
    cout << "  { \"" << scd->name << "\",\t";
    int keys = scansc(scd->def);
    cout << "0x" << hex << keys << "},\n";
    scd++;
  }
}

int main()
{
  cout << "tNamedChord chord_names[n_chord_names] = {\n";
  gensc(chords);
  cout << "};\n\n";

  cout << "tNamedChord mScaleNames[n_scale_names] = {\n";
  gensc(scales);
  cout << "};\n\n";

  return 0;
}

#endif
