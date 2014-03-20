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

#include "Synth.h"

#include "Globals.h"
#include "JazzPlusPlusApplication.h"
#include "SynthesizerTypeEnums.h"
#include "SysexChannel.h"

#include <cassert>
#include <cstdlib>
#include <string>

using namespace std;

#define SXDECL(id,len,arr) do\
{\
  sxlen[id] = len;\
  sxdata[id] = new unsigned char[len];\
  memcpy(sxdata[id], arr, len);\
} while (0)


#define GS_DT1 0x41,0x10,0x42,0x12
#define GS_DT1_LEN 10

#define XG_NAT 0x43,0x10,0x4c
#define XG_NAT_LEN 8 // no command ID or checksum for XG native!

JZSynthesizer* NewSynth(const string& Type)
{
  int i;
  for (i = 0; i < NumSynthTypes; i++)
  {
    if (gSynthesizerTypes[i].first == Type)
    {
      break;
    }
  }
  switch (i)
  {
    case SynthTypeGM:
      return new JZSynthesizerGm;
    case SynthTypeGS:
      return new JZSynthesizerGs;
    case SynthTypeXG:
      return new JZSynthesizerXg;
    case SynthTypeOther:
    default:
      return new JZSynthesizerGm;
  }
}

vector<string> JZSynthesizerSysex::mSysexNames;

vector<string> JZSynthesizerSysex::mSysexGroupNames;

const string& JZSynthesizerSysex::GetSysexName(unsigned i)
{
  if (i < mSysexNames.size())
  {
    return mSysexNames[i];
  }

  static string Unkown("UNKNOWN");
  return Unkown;
}

const string& JZSynthesizerSysex::GetSysexGroupName(unsigned i)
{
  if (i < mSysexGroupNames.size())
  {
    return mSysexGroupNames[i];
  }

  static string Unkown("UNKNOWN");
  return Unkown;
}

JZSynthesizerSysex::JZSynthesizerSysex()
{
  int i;

  for (i = 0; i < NumSysexIds; i++)
  {
    sxlen[i] = 0;
    sxdata [i] = 0;
  }

  if (mSysexNames.empty())
  {
    // Create storage.
    mSysexNames.resize(NumSysexIds);

    // Set the names.
    mSysexNames[SX_NONE] = "Unknown";

    // Misc
    mSysexNames[SX_UNIV_NON_REALTIME] = "UNIV_NON_REALTIME";
    mSysexNames[SX_UNIV_REALTIME] = "UNIV_REALTIME";
    mSysexNames[SX_ROLAND_DT1] = "ROLAND_DT1";
    mSysexNames[SX_ROLAND_RQ1] = "ROLAND_RQ1";
    mSysexNames[SX_ROLAND_UNKNOWN] = "ROLAND_UNKNOWN";
    mSysexNames[SX_XG_NATIVE] = "XG_NATIVE";
    mSysexNames[SX_MU80_NATIVE] = "MU80_NATIVE";
    mSysexNames[SX_YAMAHA_UNKNOWN] = "YAMAHA_UNKNOWN";


    // GM
    mSysexNames[SX_GM_ON] = "GM_ON";
    mSysexNames[SX_GM_MasterVol] = "GM_MasterVol";

    // GS DT1
    // 0x40 0x00 0x??:
    mSysexNames[SX_GS_ON] = "GS_ON";
    mSysexNames[SX_GS_MasterVol] = "GS_MasterVol";
    mSysexNames[SX_GS_MasterPan] = "GS_MasterPan";

    // 0x40 0x2n 0x??:
    // Must be in sequence:
    mSysexNames[SX_GS_ModPitch] = "GS_ModPitch";
    mSysexNames[SX_GS_ModTvf] = "GS_ModTvf";
    mSysexNames[SX_GS_ModAmpl] = "GS_ModAmpl";
    mSysexNames[SX_GS_ModLfo1Rate] = "GS_ModLfo1Rate";
    mSysexNames[SX_GS_ModLfo1Pitch] = "GS_ModLfo1Pitch";
    mSysexNames[SX_GS_ModLfo1Tvf] = "GS_ModLfo1Tvf";
    mSysexNames[SX_GS_ModLfo1Tva] = "GS_ModLfo1Tva";
    mSysexNames[SX_GS_ModLfo2Rate] = "GS_ModLfo2Rate";
    mSysexNames[SX_GS_ModLfo2Pitch] = "GS_ModLfo2Pitch";
    mSysexNames[SX_GS_ModLfo2Tvf] = "GS_ModLfo2Tvf";
    mSysexNames[SX_GS_ModLfo2Tva] = "GS_ModLfo2Tva";

    // Must be in sequence:
    mSysexNames[SX_GS_BendPitch] = "GS_BendPitch";
    mSysexNames[SX_GS_BendTvf] = "GS_BendTvf";
    mSysexNames[SX_GS_BendAmpl] = "GS_BendAmpl";
    mSysexNames[SX_GS_BendLfo1Rate] = "GS_BendLfo1Rate";
    mSysexNames[SX_GS_BendLfo1Pitch] = "GS_BendLfo1Pitch";
    mSysexNames[SX_GS_BendLfo1Tvf] = "GS_BendLfo1Tvf";
    mSysexNames[SX_GS_BendLfo1Tva] = "GS_BendLfo1Tva";
    mSysexNames[SX_GS_BendLfo2Rate] = "GS_BendLfo2Rate";
    mSysexNames[SX_GS_BendLfo2Pitch] = "GS_BendLfo2Pitch";
    mSysexNames[SX_GS_BendLfo2Tvf] = "GS_BendLfo2Tvf";
    mSysexNames[SX_GS_BendLfo2Tva] = "GS_BendLfo2Tva";

    // Must be in sequence:
    mSysexNames[SX_GS_CafPitch] = "GS_CafPitch";
    mSysexNames[SX_GS_CafTvf] = "GS_CafTvf";
    mSysexNames[SX_GS_CafAmpl] = "GS_CafAmpl";
    mSysexNames[SX_GS_CafLfo1Rate] = "GS_CafLfo1Rate";
    mSysexNames[SX_GS_CafLfo1Pitch] = "GS_CafLfo1Pitch";
    mSysexNames[SX_GS_CafLfo1Tvf] = "GS_CafLfo1Tvf";
    mSysexNames[SX_GS_CafLfo1Tva] = "GS_CafLfo1Tva";
    mSysexNames[SX_GS_CafLfo2Rate] = "GS_CafLfo2Rate";
    mSysexNames[SX_GS_CafLfo2Pitch] = "GS_CafLfo2Pitch";
    mSysexNames[SX_GS_CafLfo2Tvf] = "GS_CafLfo2Tvf";
    mSysexNames[SX_GS_CafLfo2Tva] = "GS_CafLfo2Tva";

    // Must be in sequence:
    mSysexNames[SX_GS_PafPitch] = "GS_PafPitch";
    mSysexNames[SX_GS_PafTvf] = "GS_PafTvf";
    mSysexNames[SX_GS_PafAmpl] = "GS_PafAmpl";
    mSysexNames[SX_GS_PafLfo1Rate] = "GS_PafLfo1Rate";
    mSysexNames[SX_GS_PafLfo1Pitch] = "GS_PafLfo1Pitch";
    mSysexNames[SX_GS_PafLfo1Tvf] = "GS_PafLfo1Tvf";
    mSysexNames[SX_GS_PafLfo1Tva] = "GS_PafLfo1Tva";
    mSysexNames[SX_GS_PafLfo2Rate] = "GS_PafLfo2Rate";
    mSysexNames[SX_GS_PafLfo2Pitch] = "GS_PafLfo2Pitch";
    mSysexNames[SX_GS_PafLfo2Tvf] = "GS_PafLfo2Tvf";
    mSysexNames[SX_GS_PafLfo2Tva] = "GS_PafLfo2Tva";

    // Must be in sequence:
    mSysexNames[SX_GS_CC1Pitch] = "GS_CC1Pitch";
    mSysexNames[SX_GS_CC1Tvf] = "GS_CC1Tvf";
    mSysexNames[SX_GS_CC1Ampl] = "GS_CC1Ampl";
    mSysexNames[SX_GS_CC1Lfo1Rate] = "GS_CC1Lfo1Rate";
    mSysexNames[SX_GS_CC1Lfo1Pitch] = "GS_CC1Lfo1Pitch";
    mSysexNames[SX_GS_CC1Lfo1Tvf] = "GS_CC1Lfo1Tvf";
    mSysexNames[SX_GS_CC1Lfo1Tva] = "GS_CC1Lfo1Tva";
    mSysexNames[SX_GS_CC1Lfo2Rate] = "GS_CC1Lfo2Rate";
    mSysexNames[SX_GS_CC1Lfo2Pitch] = "GS_CC1Lfo2Pitch";
    mSysexNames[SX_GS_CC1Lfo2Tvf] = "GS_CC1Lfo2Tvf";
    mSysexNames[SX_GS_CC1Lfo2Tva] = "GS_CC1Lfo2Tva";

    // Must be in sequence:
    mSysexNames[SX_GS_CC2Pitch] = "GS_CC2Pitch";
    mSysexNames[SX_GS_CC2Tvf] = "GS_CC2Tvf";
    mSysexNames[SX_GS_CC2Ampl] = "GS_CC2Ampl]";
    mSysexNames[SX_GS_CC2Lfo1Rate] = "GS_CC2Lfo1Rate";
    mSysexNames[SX_GS_CC2Lfo1Pitch] = "GS_CC2Lfo1Pitch";
    mSysexNames[SX_GS_CC2Lfo1Tvf] = "GS_CC2Lfo1Tvf";
    mSysexNames[SX_GS_CC2Lfo1Tva] = "GS_CC2Lfo1Tva";
    mSysexNames[SX_GS_CC2Lfo2Rate] = "GS_CC2Lfo2Rate";
    mSysexNames[SX_GS_CC2Lfo2Pitch] = "GS_CC2Lfo2Pitch";
    mSysexNames[SX_GS_CC2Lfo2Tvf] = "GS_CC2Lfo2Tvf";
    mSysexNames[SX_GS_CC2Lfo2Tva] = "GS_CC2Lfo2Tva";

    // 0x40 0x01 0x??:
    // Must be in sequence:
    mSysexNames[SX_GS_ReverbMacro] = "GS_ReverbMacro";
    mSysexNames[SX_GS_RevCharacter] = "GS_RevCharacter";
    mSysexNames[SX_GS_RevPreLpf] = "GS_RevPreLpf";
    mSysexNames[SX_GS_RevLevel] = "GS_RevLevel";
    mSysexNames[SX_GS_RevTime] = "GS_RevTime";
    mSysexNames[SX_GS_RevDelayFeedback] = "GS_RevDelayFeedback";
    mSysexNames[SX_GS_RevSendChorus] = "GS_RevSendChorus";

    // Must be in sequence:
    mSysexNames[SX_GS_ChorusMacro] = "GS_ChorusMacro";
    mSysexNames[SX_GS_ChoPreLpf] = "GS_ChoPreLpf";
    mSysexNames[SX_GS_ChoLevel] = "GS_ChoLevel";
    mSysexNames[SX_GS_ChoFeedback] = "GS_ChoFeedback";
    mSysexNames[SX_GS_ChoDelay] = "GS_ChoDelay";
    mSysexNames[SX_GS_ChoRate] = "GS_ChoRate";
    mSysexNames[SX_GS_ChoDepth] = "GS_ChoDepth";
    mSysexNames[SX_GS_ChoSendReverb] = "GS_ChoSendReverb]";

    mSysexNames[SX_GS_PartialReserve] = "GS_PartialReserve";

    // 0x40 0x1n 0x??:
    mSysexNames[SX_GS_RxChannel] = "GS_RxChannel";
    mSysexNames[SX_GS_UseForRhythm] = "GS_UseForRhythm";
    mSysexNames[SX_GS_CC1CtrlNo] = "GS_CC1CtrlNo";
    mSysexNames[SX_GS_CC2CtrlNo] = "GS_CC2CtrlNo";


    // XG
    mSysexNames[SX_XG_ON] = "XG_ON";

    // Native Multipart:
    // Must be in sequence
    mSysexNames[SX_XG_ModPitch] = "XG_ModPitch";
    mSysexNames[SX_XG_ModTvf] = "XG_ModTvf";
    mSysexNames[SX_XG_ModAmpl] = "XG_ModAmpl";
    mSysexNames[SX_XG_ModLfoPitch] = "XG_ModLfoPitch";
    mSysexNames[SX_XG_ModLfoTvf] = "XG_ModLfoTvf";
    mSysexNames[SX_XG_ModLfoTva] = "XG_ModLfoTva";
    mSysexNames[SX_XG_BendPitch] = "XG_BendPitch";
    mSysexNames[SX_XG_BendTvf] = "XG_BendTvf";
    mSysexNames[SX_XG_BendAmpl] = "XG_BendAmpl";
    mSysexNames[SX_XG_BendLfoPitch] = "XG_BendLfoPitch";
    mSysexNames[SX_XG_BendLfoTvf] = "XG_BendLfoTvf";
    mSysexNames[SX_XG_BendLfoTva] = "XG_BendLfoTva";

    // Must be in sequence:
    mSysexNames[SX_XG_CafPitch] = "XG_CafPitch";
    mSysexNames[SX_XG_CafTvf] = "XG_CafTvf";
    mSysexNames[SX_XG_CafAmpl] = "XG_CafAmpl]";
    mSysexNames[SX_XG_CafLfoPitch] = "XG_CafLfoPitch";
    mSysexNames[SX_XG_CafLfoTvf] = "XG_CafLfoTvf";
    mSysexNames[SX_XG_CafLfoTva] = "XG_CafLfoTva";
    mSysexNames[SX_XG_PafPitch] = "XG_PafPitch";
    mSysexNames[SX_XG_PafTvf] = "XG_PafTvf";
    mSysexNames[SX_XG_PafAmpl] = "XG_PafAmpl";
    mSysexNames[SX_XG_PafLfoPitch] = "XG_PafLfoPitch";
    mSysexNames[SX_XG_PafLfoTvf] = "XG_PafLfoTvf";
    mSysexNames[SX_XG_PafLfoTva] = "XG_PafLfoTva";
    mSysexNames[SX_XG_CC1CtrlNo] = "XG_CC1CtrlNo";
    mSysexNames[SX_XG_CC1Pitch] = "XG_CC1Pitch";
    mSysexNames[SX_XG_CC1Tvf] = "XG_CC1Tvf";
    mSysexNames[SX_XG_CC1Ampl] = "XG_CC1Ampl";
    mSysexNames[SX_XG_CC1LfoPitch] = "XG_CC1LfoPitch";
    mSysexNames[SX_XG_CC1LfoTvf] = "XG_CC1LfoTvf";
    mSysexNames[SX_XG_CC1LfoTva] = "XG_CC1LfoTva";
    mSysexNames[SX_XG_CC2CtrlNo] = "XG_CC2CtrlNo";
    mSysexNames[SX_XG_CC2Pitch] = "XG_CC2Pitch";
    mSysexNames[SX_XG_CC2Tvf] = "XG_CC2Tvf";
    mSysexNames[SX_XG_CC2Ampl] = "XG_CC2Ampl";
    mSysexNames[SX_XG_CC2LfoPitch] = "XG_CC2LfoPitch";
    mSysexNames[SX_XG_CC2LfoTvf] = "XG_CC2LfoTvf";
    mSysexNames[SX_XG_CC2LfoTva] = "XG_CC2LfoTva";



    mSysexNames[SX_XG_ReverbMacro] = "XG_ReverbMacro";
    mSysexNames[SX_XG_ChorusMacro] = "XG_ChorusMacro";
    mSysexNames[SX_XG_EqualizerMacro] = "XG_EqualizerMacro";

    mSysexNames[SX_XG_RxChannel] = "XG_RxChannel";
    mSysexNames[SX_XG_UseForRhythm] = "XG_UseForRhythm";
  }

  if (mSysexGroupNames.empty())
  {
    // Create storage.
    mSysexGroupNames.resize(130);

    // Set the names.
    mSysexGroupNames[SX_GROUP_UNKNOWN + 1] = "Other Sysex";
    mSysexGroupNames[SX_GROUP_GM + 1] = "GM Sysex";
    mSysexGroupNames[SX_GROUP_GS + 1] = "GS Sysex";
    mSysexGroupNames[SX_GROUP_XG + 1] = "XG Sysex";
  }

  // GM
  unsigned char gm_on[] = {0x7e,0x7f,0x09,0x01,0xf7};
  SXDECL(SX_GM_ON, 5, gm_on);

  unsigned char gm_master_vol[] = {0x7f,0x7f,0x04,0x01,0x00,0x00,0xf7};
  SXDECL(SX_GM_MasterVol, 7, gm_master_vol);


  // GS DT1
  unsigned char gs_on[] = {GS_DT1,0x40,0x00,0x7f,0x00,0x41,0xf7};
  SXDECL(SX_GS_ON,GS_DT1_LEN, gs_on);

  unsigned char gs_dt1[] = {GS_DT1,0x40,0x00,0x04,0x00,0x00,0xf7};

  gs_dt1[6] = 0x04;
  SXDECL(SX_GS_MasterVol, GS_DT1_LEN, gs_dt1);

  gs_dt1[6] = 0x06;
  SXDECL(SX_GS_MasterPan, GS_DT1_LEN, gs_dt1);

  gs_dt1[4] = 0x40;
  gs_dt1[5] = 0x20;
  for (i = 0; i < 11; i++)
  {
    gs_dt1[6] = 0x00 + i;
    SXDECL(SX_GS_ModPitch + i, GS_DT1_LEN, gs_dt1);
  }

  for (i = 0; i < 11; i++)
  {
    gs_dt1[6] = 0x10 + i;
    SXDECL(SX_GS_BendPitch + i, GS_DT1_LEN, gs_dt1);
  }

  for (i = 0; i < 11; i++)
  {
    gs_dt1[6] = 0x20 + i;
    SXDECL(SX_GS_CafPitch + i, GS_DT1_LEN, gs_dt1);
  }

  for (i = 0; i < 11; i++)
  {
    gs_dt1[6] = 0x30 + i;
    SXDECL(SX_GS_PafPitch + i, GS_DT1_LEN, gs_dt1);
  }

  for (i = 0; i < 11; i++)
  {
    gs_dt1[6] = 0x40 + i;
    SXDECL(SX_GS_CC1Pitch + i, GS_DT1_LEN, gs_dt1);
  }

  for (i = 0; i < 11; i++)
  {
    gs_dt1[6] = 0x50 + i;
    SXDECL(SX_GS_CC2Pitch + i, GS_DT1_LEN, gs_dt1);
  }

  gs_dt1[4] = 0x40;
  gs_dt1[5] = 0x01;
  for (i = 0; i < 7; i++)
  {
    gs_dt1[6] = 0x30 + i;
    SXDECL(SX_GS_ReverbMacro + i, GS_DT1_LEN, gs_dt1);
  }

  for (i = 0; i < 8; i++)
  {
    gs_dt1[6] = 0x38 + i;
    SXDECL(SX_GS_ChorusMacro + i, GS_DT1_LEN, gs_dt1);
  }

  const unsigned char gs_partial_reserve[] =
  {
    GS_DT1,
    0x40,
    0x01,
    0x10,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0xf7
  };

  SXDECL(SX_GS_PartialReserve, GS_DT1_LEN, gs_partial_reserve);

  gs_dt1[4] = 0x40;
  gs_dt1[5] = 0x10;
  gs_dt1[6] = 0x02;
  SXDECL(SX_GS_RxChannel, GS_DT1_LEN, gs_dt1);

  gs_dt1[4] = 0x40;
  gs_dt1[5] = 0x10;
  gs_dt1[6] = 0x15;
  SXDECL(SX_GS_UseForRhythm, GS_DT1_LEN, gs_dt1);

  gs_dt1[4] = 0x40;
  gs_dt1[5] = 0x10;
  gs_dt1[6] = 0x1f;
  SXDECL(SX_GS_CC1CtrlNo, GS_DT1_LEN, gs_dt1);

  gs_dt1[4] = 0x40;
  gs_dt1[5] = 0x10;
  gs_dt1[6] = 0x20;
  SXDECL(SX_GS_CC2CtrlNo, GS_DT1_LEN, gs_dt1);

  // XG native
  const unsigned char xg_on[] = {XG_NAT,0x00,0x00,0x7e,0x00,0xf7};
  SXDECL(SX_XG_ON, XG_NAT_LEN, xg_on);

  // XG native multipart
  unsigned char xg_multi[] = {XG_NAT,0x08,0x00,0x00,0x00,0x7f};

  for (i = 0; i < 12; i++)
  {
    xg_multi[5] = 0x1d + i;
    SXDECL(SX_XG_ModPitch + i, XG_NAT_LEN, xg_multi);
  }

  for (i = 0; i < 26; i++)
  {
    xg_multi[5] = 0x4d + i;
    SXDECL(SX_XG_CafPitch + i, XG_NAT_LEN, xg_multi);
  }

  xg_multi[5] = 0x04;
  SXDECL(SX_XG_RxChannel, XG_NAT_LEN, xg_multi);

  xg_multi[5] = 0x07;
  SXDECL(SX_XG_UseForRhythm, XG_NAT_LEN, xg_multi);


  // XG native effect 1
  unsigned char xg_effect1_2[] = {XG_NAT,0x02,0x01,0x00,0x00,0x00,0x7f};

  xg_effect1_2[5] = 0x00;
  SXDECL(SX_XG_ReverbMacro, XG_NAT_LEN, xg_effect1_2);

  xg_effect1_2[5] = 0x20;
  SXDECL(SX_XG_ChorusMacro, XG_NAT_LEN, xg_effect1_2);

  // XG multi EQ
  unsigned char xg_multiEQ[] = {XG_NAT,0x02,0x40,0x00,0x00,0x00,0x7f};

  xg_multiEQ[5] = 0x00;
  SXDECL(SX_XG_EqualizerMacro, XG_NAT_LEN, xg_multiEQ);
}

JZSynthesizerSysex::~JZSynthesizerSysex()
{
  for (int i = 0; i < NumSysexIds; i++)
  {
    delete sxdata[i];
  }
}

int JZSynthesizerSysex::GetId(const JZSysExEvent* pSysEx) const
{
  if (!pSysEx)
  {
    return SX_NONE;
  }

  const unsigned char* pData = pSysEx->GetData();
  switch (pData[0])
  {
    case 0x7e:
      // GM ON ?
      if (!memcmp(sxdata[SX_GM_ON], pData, pSysEx->GetDataLength()))
      {
        return SX_GM_ON;
      }
      else
      {
        return SX_UNIV_NON_REALTIME;
      }
      break;

    case 0x7f:
      // GM MasterVol ?
      if (!memcmp(sxdata[SX_GM_MasterVol], pData, 4))
      {
        return SX_GM_MasterVol;
      }
      else
      {
        return SX_UNIV_REALTIME;
      }
      break;

    case 0x41:
      //-------
      // Roland
      //-------

      // GS DT1?
      if (pData[2] == 0x42 && pData[3] == 0x12)
      {
        register unsigned char a1 = pData[4];
        register unsigned char a2 = pData[5];
        register unsigned char a3 = pData[6];

        if (a1 == 0x40)
        {
          // MSB address 0x40:
          if (a2 == 0x00)
          {
            // 0x40 0x00 0x??
            switch (a3)
            {
              case 0x7f:
                return SX_GS_ON;
              case 0x04:
                return SX_GS_MasterVol;
              case 0x06:
                return SX_GS_MasterPan;
              default:
                break;
            }
          }
          else if (a2 == 0x01)
          {
            // 0x40 0x01 0x??
            if (a3 >= 0x30 && a3 <= 0x36)
            {
              // These are reverb settings.
              return SX_GS_ReverbMacro + (a3 - 0x30);
            }
            else if (a3 >= 0x38 && a3 <= 0x3f)
            {
              // These are chorus settings.
              return SX_GS_ChorusMacro + (a3 - 0x38);
            }
            else if (a3 == 0x10)
            {
              return SX_GS_PartialReserve;
            }
          }
          else if ((a2 & 0xf0) == 0x10)
          {
            // 0x40 0x1n 0x??
            switch (a3)
            {
              case 0x02:
                return SX_GS_RxChannel;

              case 0x15:
                return SX_GS_UseForRhythm;

              case 0x1f:
                return SX_GS_CC1CtrlNo;

              case 0x20:
                return SX_GS_CC2CtrlNo;

              default:
                break;
            }
          }
          else if ((a2 & 0xf0) == 0x20)
          {
            // 0x40 0x2n 0x??
            if (a3 <= 0x0a)
            {
              return SX_GS_ModPitch + (a3 - 0x00);
            }
            else if (a3 >= 0x10 && a3 <= 0x1a)
            {
              return SX_GS_BendPitch + (a3 - 0x10);
            }
            else if (a3 >= 0x20 && a3 <= 0x2a)
            {
              return SX_GS_CafPitch + (a3 - 0x20);
            }
            else if (a3 >= 0x30 && a3 <= 0x3a)
            {
              return SX_GS_PafPitch + (a3 - 0x30);
            }
            else if (a3 >= 0x40 && a3 <= 0x4a)
            {
              return SX_GS_CC1Pitch + (a3 - 0x40);
            }
            else if (a3 >= 0x50 && a3 <= 0x5a)
            {
              return SX_GS_CC2Pitch + (a3 - 0x50);
            }
          }
        } // end a1 == 0x40
      } // end GS DT1

      if (pData[3] == 0x12 && pSysEx->GetDataLength() >= 10)
      {
        return SX_ROLAND_DT1;
      }
      else if (pData[3] == 0x11 && pSysEx->GetDataLength() >= 12)
      {
        return SX_ROLAND_RQ1;
      }
      else
      {
        return SX_ROLAND_UNKNOWN;
      }

      break;
      // end Roland

    case 0x43:
      //-------
      // Yamaha
      //-------

      // XG Native?
      if (((pData[1] & 0xf0) == 0x10) && pData[2] == 0x4c)
      {
        register unsigned char a1 = pData[3];
        register unsigned char a2 = pData[4];
        register unsigned char a3 = pData[5];

        // Multipart?
        if (a1 == 0x08)
        {
          if (a3 >= 0x1d && a3 <= 0x28)
          {
            return SX_XG_ModPitch + (a3 - 0x1d);
          }
          else if (a3 >= 0x4d && a3 <= 0x66)
          {
            return SX_XG_CafPitch + (a3 - 0x4d);
          }
          else if (a3 == 0x04)
          {
            return SX_XG_RxChannel;
          }
          else if (a3 == 0x07)
          {
            return SX_XG_UseForRhythm;
          }
        }

        // Effect 1?
        else if (a1 == 0x02 && a2 == 0x01)
        {
          if (a3 == 0x00)
          {
            return SX_XG_ReverbMacro;
          }
          else if (a3 == 0x20)
          {
            return SX_XG_ChorusMacro;
          }
        }

        // Multi EQ?
        else if (a1 == 0x02 && a2 == 0x40)
        {
          if (a3 == 0x00)
          {
            return SX_XG_EqualizerMacro;
          }
        }

        // XG system on?
        else if (a1 == 0x00 && a2 == 0x00 && a3 == 0x7e)
        {
          return SX_XG_ON;
        }
      }

      if (pData[2] == 0x4c)
      {
        return SX_XG_NATIVE;
      }
      else if (pData[2] == 0x49)
      {
        return SX_MU80_NATIVE;
      }
      else
      {
        return SX_YAMAHA_UNKNOWN;
      }

      break;
      // end Yamaha

    default:
      break;
  }

  // Not recognized
  return SX_NONE;
}


const unsigned char* JZSynthesizerSysex::GetValPtr(const JZSysExEvent* pSysEx) const
{
  if (!pSysEx)
  {
    return 0;
  }

  const unsigned char* pData = pSysEx->GetData();
  switch (pData[0])
  {
    case 0x7f:
      // GM MasterVol?
      if (!memcmp(sxdata[SX_GM_MasterVol], pData, 4))
      {
        return &pData[4];
      }
      break;

    case 0x41:
      //-------
      // Roland
      //-------

      // GS DT1?
      if (pData[2] == 0x42 && pData[3] == 0x12 && pSysEx->GetDataLength() >= 10)
      {
        return &pData[7];
      }
      // other DT1 or RQ1 ?
      else if (
        (pData[3] == 0x12 && pSysEx->GetDataLength() >= 10) ||
        (pData[3] == 0x11 && pSysEx->GetDataLength() >= 12))
      {
        return &pData[7];
      }
      break;

    case 0x43:
      // Yamaha!
      // XG Native?
      if (((pData[1] & 0xf0) == 0x10) && pData[2] == 0x4c)
      {
        return &pData[6];
      }
      break;

    default:
      break;
  }

  // Not recognized
  return 0;
}

//-----------------------------------------------------------------------------
// Description:
//   Return a pointer to the byte with the channel (if any).
//-----------------------------------------------------------------------------
const unsigned char* JZSynthesizerSysex::GetChaPtr(const JZSysExEvent* pSysEx)
{
  if (!pSysEx)
  {
    return 0;
  }

  const unsigned char* pData = pSysEx->GetData();
  switch (pData[0])
  {
    case 0x41:
      //-------
      // Roland
      //-------

      // GS DT1 + address 0x40?
      if (pData[2] == 0x42 && pData[3] == 0x12 && pData[4] == 0x40)
      {
        if ((pData[5] & 0xf0) == 0x10 || (pData[5] & 0xf0) == 0x20)
        {
          return &pData[5];
        }
      }
      break;

    case 0x43:
      //-------
      // Yamaha
      //-------

      // XG Native multipart?
      if ((pData[1] & 0xf0) == 0x10 && pData[2] == 0x4c && pData[3] == 0x08)
      {
        return &pData[4];
      }
      break;
    default:
      break;
  }

  // Not recognized.
  return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSynthesizerSysex::FixCheckSum(JZSysExEvent* pSysEx)
{
  const unsigned char* pData = pSysEx->GetData();
  if (
    pData[0] == 0x41 &&
    ((pData[3] == 0x12 && pSysEx->GetDataLength() >= 10) ||
     (pData[3] == 0x11 && pSysEx->GetDataLength() >= 12)))
  {
    // The synthesizer is a Roland RQ1 or DT1.
    pSysEx->FixCheckSum();
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZSysExEvent* JZSynthesizerSysex::operator()(long clk, int id, unsigned char val)
{
  return (*this)(clk, id, -1, 1, &val);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZSysExEvent* JZSynthesizerSysex::operator()(
  long clk,
  int id,
  int datalen,
  unsigned char val[])
{
  return (*this)(clk, id, -1, datalen, val);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZSysExEvent* JZSynthesizerSysex::operator()(
  long clk,
  int id,
  int channel,
  int datalen,
  unsigned char val[])
{
  assert((id > SX_NONE) && (id < NumSysexIds));
  assert(datalen > 0);

  int i;
  for (i = 0; i < datalen; i++)
  {
    assert(val[i] < 128);
  }

   int len = sxlen[id] + datalen - 1;
   unsigned char* sx = new unsigned char[len];
   memcpy(sx, sxdata[id], sxlen[id]);
   JZSysExEvent* pSysEx = 0;

   if (id == SX_GM_MasterVol)
   {
      // GM
      if (datalen == 2)
        sx[4] = val[1]; // LSB is second byte!
      else
        sx[4] = 0;
      sx[5] = val[0]; // MSB
      pSysEx = new JZSysExEvent(clk, sx, len);
   }
   else if ((id > SX_GS_ON) && (id < SX_XG_ON))
   {
      // GS DT1
      for (i = 0; i < datalen; i++)
        sx[i+7] = val[i];

      if (channel >= 0)
      {
        sx[5] = sx[5] | sysex_channel(channel);
      }

      unsigned char sum = 0x00;
      for (i = 4; i < (len-2); i++)
        sum += sx[i];
      sx[len - 2] = (0x80 - (sum & 0x7f)) & 0x7f;
      sx[len-1] = 0xf7;
      pSysEx = new JZSysExEvent(clk, sx, len);
   }
   else if (id > SX_XG_ON)
   {
      // XG Native
      for (i = 0; i < datalen; i++)
      {
        sx[i+6] = val[i];
      }

      if (channel >= 0)
      {
        sx[4] = channel - 1;
      }
      sx[len-1] = 0xf7;
      pSysEx = new JZSysExEvent(clk, sx, len);
   }

   delete sx;
   return pSysEx;
}


JZEvent* JZSynthesizerGs::MasterVolSX(long clk, unsigned char vol)
{
   return Sysex(clk, SX_GS_MasterVol, vol);
}

JZEvent* JZSynthesizerGs::MasterPanSX(long clk, unsigned char pan)
{
   return Sysex(clk, SX_GS_MasterPan, pan);
}

JZEvent* JZSynthesizerGs::ModSX(int index, long clk, int cha, unsigned char val)
{
   return Sysex(clk, SX_GS_ModPitch + index, cha, 1, &val);
}

JZEvent* JZSynthesizerGs::BendSX(int index, long clk, int cha, unsigned char val)
{
   return Sysex(clk, SX_GS_BendPitch + index, cha, 1, &val);
}

JZEvent* JZSynthesizerGs::CafSX(int index, long clk, int cha, unsigned char val)
{
   return Sysex(clk, SX_GS_CafPitch + index, cha, 1, &val);
}

JZEvent* JZSynthesizerGs::PafSX(int index, long clk, int cha, unsigned char val)
{
   return Sysex(clk, SX_GS_PafPitch + index, cha, 1, &val);
}

JZEvent* JZSynthesizerGs::CC1SX(int index, long clk, int cha, unsigned char val)
{
   return Sysex(clk, SX_GS_CC1Pitch + index, cha, 1, &val);
}

JZEvent* JZSynthesizerGs::CC2SX(int index, long clk, int cha, unsigned char val)
{
   return Sysex(clk, SX_GS_CC2Pitch + index, cha, 1, &val);
}

JZEvent* JZSynthesizerGs::PartialReserveSX(long clk, int cha, unsigned char *valptr)
{
   return Sysex(clk, SX_GS_PartialReserve, 16, valptr);
}

JZEvent* JZSynthesizerGs::RxChannelSX(long clk, int cha, unsigned char val)
{
   return Sysex(clk, SX_GS_RxChannel, cha, 1, &val);
}

JZEvent* JZSynthesizerGs::UseForRhythmSX(long clk, int cha, unsigned char val)
{
   return Sysex(clk, SX_GS_UseForRhythm, cha, 1, &val);
}

JZEvent* JZSynthesizerGs::ControllerNumberSX(int ctrlno, long clk, int cha, unsigned char val)
{
  if (ctrlno == 1)
    return Sysex(clk, SX_GS_CC1CtrlNo, cha, 1, &val);
  else if (ctrlno == 2)
    return Sysex(clk, SX_GS_CC2CtrlNo, cha, 1, &val);
  else
    return 0;
}

JZEvent* JZSynthesizerGs::ReverbMacroSX(long clk, unsigned char val, unsigned char lsb)
{
   return Sysex(clk, SX_GS_ReverbMacro, val);
}

JZEvent* JZSynthesizerGs::ReverbParamSX(int index, long clk, unsigned char val)
{
   return Sysex(clk, SX_GS_RevCharacter + index, val);
}

JZEvent* JZSynthesizerGs::ChorusMacroSX(long clk, unsigned char val, unsigned char lsb)
{
   return Sysex(clk, SX_GS_ChorusMacro, val);
}

JZEvent* JZSynthesizerGs::ChorusParamSX(int index, long clk, unsigned char val)
{
   return Sysex(clk, SX_GS_ChoPreLpf + index, val);
}



// XG:

JZEvent* JZSynthesizerXg::ModSX(int index, long clk, int cha, unsigned char val)
{
   if ((index >= 0) && (index <= 2))
      return Sysex(clk, SX_XG_ModPitch + index, cha, 1, &val);
   else if ((index >= 4) && (index <=6))
      return Sysex(clk, SX_XG_ModPitch + index - 1, cha, 1, &val);
   else
      return 0;
}

JZEvent* JZSynthesizerXg::BendSX(int index, long clk, int cha, unsigned char val)
{
   if ((index >= 0) && (index <= 2))
      return Sysex(clk, SX_XG_BendPitch + index, cha, 1, &val);
   else if ((index >= 4) && (index <=6))
       return Sysex(clk, SX_XG_BendPitch + index - 1, cha, 1, &val);
   else
      return 0;
}

JZEvent* JZSynthesizerXg::CafSX(int index, long clk, int cha, unsigned char val)
{
   if ((index >= 0) && (index <= 2))
      return Sysex(clk, SX_XG_CafPitch + index, cha, 1, &val);
   else if ((index >= 4) && (index <= 6))
       return Sysex(clk, SX_XG_CafPitch + index - 1, cha, 1, &val);
   else
      return 0;
}

JZEvent* JZSynthesizerXg::PafSX(int index, long clk, int cha, unsigned char val)
{
   if ((index >= 0) && (index <= 2))
      return Sysex(clk, SX_XG_PafPitch + index, cha, 1, &val);
   else if ((index >= 4) && (index <= 6))
       return Sysex(clk, SX_XG_PafPitch + index - 1, cha, 1, &val);
   else
      return 0;
}

JZEvent* JZSynthesizerXg::CC1SX(int index, long clk, int cha, unsigned char val)
{
   if ((index >= 0) && (index <= 2))
      return Sysex(clk, SX_XG_CC1Pitch + index, cha, 1, &val);
   else if ((index >= 4) && (index <= 6))
       return Sysex(clk, SX_XG_CC1Pitch + index - 1, cha, 1, &val);
   else
      return 0;
}

JZEvent* JZSynthesizerXg::CC2SX(int index, long clk, int cha, unsigned char val)
{
   if ((index >= 0) && (index <= 2))
      return Sysex(clk, SX_XG_CC2Pitch + index, cha, 1, &val);
   else if ((index >= 4) && (index <= 6))
       return Sysex(clk, SX_XG_CC2Pitch + index - 1, cha, 1, &val);
   else
      return 0;
}

JZEvent* JZSynthesizerXg::RxChannelSX(long clk, int cha, unsigned char val)
{
   return Sysex(clk, SX_XG_RxChannel, cha, 1, &val);
}

JZEvent* JZSynthesizerXg::UseForRhythmSX(long clk, int cha, unsigned char val)
{
   return Sysex(clk, SX_XG_UseForRhythm, cha, 1, &val);
}

JZEvent* JZSynthesizerXg::ControllerNumberSX(int ctrlno, long clk, int cha, unsigned char val)
{
   if (ctrlno == 1)
      return Sysex(clk, SX_XG_CC1CtrlNo, cha, 1, &val);
   else if (ctrlno == 2)
      return Sysex(clk, SX_XG_CC2CtrlNo, cha, 1, &val);
   else
      return 0;
}

JZEvent* JZSynthesizerXg::ReverbMacroSX(long clk, unsigned char val, unsigned char lsb)
{
   unsigned char valp[2];
   valp[0] = val;
   valp[1] = lsb;

   return Sysex(clk, SX_XG_ReverbMacro, 2, valp);
}

JZEvent* JZSynthesizerXg::ChorusMacroSX(long clk, unsigned char val, unsigned char lsb)
{
  unsigned char valp[2];
  valp[0] = val;
  valp[1] = lsb;

  return Sysex(clk, SX_XG_ChorusMacro, 2, valp);
}

JZEvent* JZSynthesizerXg::EqualizerMacroSX(long clk, unsigned char val)
{
  return Sysex(clk, SX_XG_EqualizerMacro, val);
}
