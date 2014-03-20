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

#include "NamedValue.h"

#include "Events.h"

#include <vector>
#include <string>

enum SysexId
{
  SX_NONE = 0,

  // Misc:
  SX_UNIV_NON_REALTIME,
  SX_UNIV_REALTIME,
  SX_ROLAND_DT1,
  SX_ROLAND_RQ1,
  SX_ROLAND_UNKNOWN,
  SX_XG_NATIVE,
  SX_MU80_NATIVE,
  SX_YAMAHA_UNKNOWN,

  // GM
  SX_GM_ON,
  SX_GM_MasterVol,

  // GS DT1
  // 0x40 0x00 0x??:
  SX_GS_ON,
  SX_GS_MasterVol,
  SX_GS_MasterPan,

  // 0x40 0x2n 0x??:
  // Must be in sequence:
  SX_GS_ModPitch,
  SX_GS_ModTvf,
  SX_GS_ModAmpl,
  SX_GS_ModLfo1Rate,
  SX_GS_ModLfo1Pitch,
  SX_GS_ModLfo1Tvf,
  SX_GS_ModLfo1Tva,
  SX_GS_ModLfo2Rate,
  SX_GS_ModLfo2Pitch,
  SX_GS_ModLfo2Tvf,
  SX_GS_ModLfo2Tva,

  // Must be in sequence:
  SX_GS_BendPitch,
  SX_GS_BendTvf,
  SX_GS_BendAmpl,
  SX_GS_BendLfo1Rate,
  SX_GS_BendLfo1Pitch,
  SX_GS_BendLfo1Tvf,
  SX_GS_BendLfo1Tva,
  SX_GS_BendLfo2Rate,
  SX_GS_BendLfo2Pitch,
  SX_GS_BendLfo2Tvf,
  SX_GS_BendLfo2Tva,

  // Must be in sequence:
  SX_GS_CafPitch,
  SX_GS_CafTvf,
  SX_GS_CafAmpl,
  SX_GS_CafLfo1Rate,
  SX_GS_CafLfo1Pitch,
  SX_GS_CafLfo1Tvf,
  SX_GS_CafLfo1Tva,
  SX_GS_CafLfo2Rate,
  SX_GS_CafLfo2Pitch,
  SX_GS_CafLfo2Tvf,
  SX_GS_CafLfo2Tva,

  // Must be in sequence:
  SX_GS_PafPitch,
  SX_GS_PafTvf,
  SX_GS_PafAmpl,
  SX_GS_PafLfo1Rate,
  SX_GS_PafLfo1Pitch,
  SX_GS_PafLfo1Tvf,
  SX_GS_PafLfo1Tva,
  SX_GS_PafLfo2Rate,
  SX_GS_PafLfo2Pitch,
  SX_GS_PafLfo2Tvf,
  SX_GS_PafLfo2Tva,

  // Must be in sequence:
  SX_GS_CC1Pitch,
  SX_GS_CC1Tvf,
  SX_GS_CC1Ampl,
  SX_GS_CC1Lfo1Rate,
  SX_GS_CC1Lfo1Pitch,
  SX_GS_CC1Lfo1Tvf,
  SX_GS_CC1Lfo1Tva,
  SX_GS_CC1Lfo2Rate,
  SX_GS_CC1Lfo2Pitch,
  SX_GS_CC1Lfo2Tvf,
  SX_GS_CC1Lfo2Tva,

  // Must be in sequence:
  SX_GS_CC2Pitch,
  SX_GS_CC2Tvf,
  SX_GS_CC2Ampl,
  SX_GS_CC2Lfo1Rate,
  SX_GS_CC2Lfo1Pitch,
  SX_GS_CC2Lfo1Tvf,
  SX_GS_CC2Lfo1Tva,
  SX_GS_CC2Lfo2Rate,
  SX_GS_CC2Lfo2Pitch,
  SX_GS_CC2Lfo2Tvf,
  SX_GS_CC2Lfo2Tva,

  // 0x40 0x01 0x??:
  // Must be in sequence:
  SX_GS_ReverbMacro,
  SX_GS_RevCharacter,
  SX_GS_RevPreLpf,
  SX_GS_RevLevel,
  SX_GS_RevTime,
  SX_GS_RevDelayFeedback,
  SX_GS_RevSendChorus,

  // Must be in sequence:
  SX_GS_ChorusMacro,
  SX_GS_ChoPreLpf,
  SX_GS_ChoLevel,
  SX_GS_ChoFeedback,
  SX_GS_ChoDelay,
  SX_GS_ChoRate,
  SX_GS_ChoDepth,
  SX_GS_ChoSendReverb,

  SX_GS_PartialReserve,

  // 0x40 0x1n 0x??:
  SX_GS_RxChannel,
  SX_GS_UseForRhythm,
  SX_GS_CC1CtrlNo,
  SX_GS_CC2CtrlNo,


  // XG
  SX_XG_ON,

  // Native Multipart:
  // Must be in sequence
  SX_XG_ModPitch,
  SX_XG_ModTvf,
  SX_XG_ModAmpl,
  SX_XG_ModLfoPitch,
  SX_XG_ModLfoTvf,
  SX_XG_ModLfoTva,
  SX_XG_BendPitch,
  SX_XG_BendTvf,
  SX_XG_BendAmpl,
  SX_XG_BendLfoPitch,
  SX_XG_BendLfoTvf,
  SX_XG_BendLfoTva,

  // Must be in sequence:
  SX_XG_CafPitch,
  SX_XG_CafTvf,
  SX_XG_CafAmpl,
  SX_XG_CafLfoPitch,
  SX_XG_CafLfoTvf,
  SX_XG_CafLfoTva,
  SX_XG_PafPitch,
  SX_XG_PafTvf,
  SX_XG_PafAmpl,
  SX_XG_PafLfoPitch,
  SX_XG_PafLfoTvf,
  SX_XG_PafLfoTva,
  SX_XG_CC1CtrlNo,
  SX_XG_CC1Pitch,
  SX_XG_CC1Tvf,
  SX_XG_CC1Ampl,
  SX_XG_CC1LfoPitch,
  SX_XG_CC1LfoTvf,
  SX_XG_CC1LfoTva,
  SX_XG_CC2CtrlNo,
  SX_XG_CC2Pitch,
  SX_XG_CC2Tvf,
  SX_XG_CC2Ampl,
  SX_XG_CC2LfoPitch,
  SX_XG_CC2LfoTvf,
  SX_XG_CC2LfoTva,



  SX_XG_ReverbMacro,
  SX_XG_ChorusMacro,
  SX_XG_EqualizerMacro,

  SX_XG_RxChannel,
  SX_XG_UseForRhythm,

  NumSysexIds
};

#define SX_GROUP_UNKNOWN 64
#define SX_GROUP_GM 65
#define SX_GROUP_GS 66
#define SX_GROUP_XG 67

class JZSynthesizerSysex
{
  public:

    JZSynthesizerSysex();
    virtual ~JZSynthesizerSysex();

    // Find out what kind of sysex this is
    int GetId(const JZSysExEvent* s) const;

    // Get pointer to the data value (if any)
    const unsigned char* GetValPtr(const JZSysExEvent* pSysEx) const;

    // Description:
    //   Return a pointer to the byte with the channel (if any).
    const unsigned char* GetChaPtr(const JZSysExEvent* pSysEx);

    // Fix checksum byte (if any)
    void FixCheckSum(JZSysExEvent* s);

    // Constant sysexes like e.g. GM Midi On
    JZSysExEvent* operator()(long clk, int id)
    {
      assert((id >= 0) && (id < NumSysexIds));
      return new JZSysExEvent(clk, sxdata[id], sxlen[id]);
    }

    // Variable sysexes like e.g. MasterVol or DT1
    JZSysExEvent* operator()(long clk, int id, unsigned char val);
    JZSysExEvent* operator()(long clk, int id, int datalen, unsigned char val[]);
    JZSysExEvent* operator()(
      long clk,
      int id,
      int channel,
      int datalen,
      unsigned char val[]);

    static const std::string& GetSysexName(unsigned i);
    static const std::string& GetSysexGroupName(unsigned i);

  private:

    static std::vector<std::string> mSysexNames;
    static std::vector<std::string> mSysexGroupNames;

    unsigned char sxlen[NumSysexIds];
    unsigned char* sxdata[NumSysexIds];
};

class JZSynthesizerGm;
class JZSynthesizerGs;
class JZSynthesizerXg;

class JZSynthesizer
{
  public:

    virtual ~JZSynthesizer()
    {
    }

    virtual const JZSynthesizerGm* IsGM() const
    {
      return 0;
    }

    virtual const JZSynthesizerGs* IsGS() const
    {
      return 0;
    }

    virtual const JZSynthesizerXg* IsXG() const
    {
      return 0;
    }

    virtual int GetSysexId(const JZSysExEvent* s) const
    {
      return Sysex.GetId(s);
    }

    virtual const unsigned char* GetSysexValPtr(JZSysExEvent* pSysEx) const
    {
      return Sysex.GetValPtr(pSysEx);
    }

    virtual const unsigned char* GetSysexChaPtr(JZSysExEvent* pSysEx)
    {
      return Sysex.GetChaPtr(pSysEx);
    }

    virtual void FixSysexCheckSum(JZSysExEvent *s)
    {
      Sysex.FixCheckSum(s);
    }

    virtual JZEvent* CreateResetEvent() = 0;

    virtual JZEvent* MasterVolSX(long clk, unsigned char vol)
    {
      return Sysex(clk, SX_GM_MasterVol, vol);
    }

    virtual JZEvent* MasterPanSX(long clk, unsigned char pan)
    {
      return 0;
    }

    virtual JZEvent* ModSX(int index, long clk, int cha, unsigned char val)
    {
      return 0;
    }

    virtual JZEvent* BendSX(int index, long clk, int cha, unsigned char val)
    {
      return 0;
    }

    virtual JZEvent* CafSX(int index, long clk, int cha, unsigned char val)
    {
      return 0;
    }

    virtual JZEvent* PafSX(int index, long clk, int cha, unsigned char val)
    {
      return 0;
    }

    virtual JZEvent* CC1SX(int index, long clk, int cha, unsigned char val)
    {
      return 0;
    }

    virtual JZEvent* CC2SX(int index, long clk, int cha, unsigned char val)
    {
      return 0;
    }

    virtual JZEvent* PartialReserveSX(long clk, int cha, unsigned char *valptr)
    {
      return 0;
    }
    virtual JZEvent* RxChannelSX(long clk, int cha, unsigned char val)
    {
      return 0;
    }
    virtual JZEvent* UseForRhythmSX(long clk, int cha, unsigned char val)
    {
      return 0;
    }

    virtual JZEvent* ControllerNumberSX(int ctrlno, long clk, int cha, unsigned char val)
    {
      return 0;
    }

    virtual JZEvent* ReverbMacroSX(long clk, unsigned char val, unsigned char lsb = 0)
    {
      return 0;
    }
    virtual JZEvent* ReverbParamSX(int index, long clk, unsigned char val)
    {
      return 0;
    }
    virtual JZEvent* ChorusMacroSX(long clk, unsigned char val, unsigned char lsb = 0)
    {
      return 0;
    }
    virtual JZEvent* ChorusParamSX(int index, long clk, unsigned char val)
    {
      return 0; }

    virtual JZEvent* EqualizerMacroSX(long clk, unsigned char val)
    {
      return 0;
    }

  protected:

    JZSynthesizerSysex Sysex;

};

JZSynthesizer* NewSynth(const std::string& Type);

class JZSynthesizerGm : public JZSynthesizer
{
  public:
    virtual const JZSynthesizerGm* IsGM() const
    {
      return this;
    }

    virtual JZEvent* CreateResetEvent()
    {
      return Sysex(0, SX_GM_ON);
    }
};

class JZSynthesizerGs : public JZSynthesizer
{
  public:
    virtual const JZSynthesizerGs* IsGS() const
    {
      return this;
    }

    virtual JZEvent* CreateResetEvent()
    {
      return Sysex(0, SX_GS_ON);
    }

    virtual JZEvent* MasterVolSX(long clk, unsigned char vol);
    virtual JZEvent* MasterPanSX(long clk, unsigned char pan);
    virtual JZEvent* ModSX(int index, long clk, int cha, unsigned char val);
    virtual JZEvent* BendSX(int index, long clk, int cha, unsigned char val);
    virtual JZEvent* CafSX(int index, long clk, int cha, unsigned char val);
    virtual JZEvent* PafSX(int index, long clk, int cha, unsigned char val);
    virtual JZEvent* CC1SX(int index, long clk, int cha, unsigned char val);
    virtual JZEvent* CC2SX(int index, long clk, int cha, unsigned char val);
    virtual JZEvent* PartialReserveSX(long clk, int cha, unsigned char *valptr);
    virtual JZEvent* RxChannelSX(long clk, int cha, unsigned char val);
    virtual JZEvent* UseForRhythmSX(long clk, int cha, unsigned char val);
    virtual JZEvent* ControllerNumberSX(int ctrlno, long clk, int cha, unsigned char val);
    virtual JZEvent* ReverbMacroSX(long clk, unsigned char val, unsigned char lsb = 0);
    virtual JZEvent* ReverbParamSX(int index, long clk, unsigned char val);
    virtual JZEvent* ChorusMacroSX(long clk, unsigned char val, unsigned char lsb = 0);
    virtual JZEvent* ChorusParamSX(int index, long clk, unsigned char val);
};

class JZSynthesizerXg : public JZSynthesizer
{
  public:
    virtual const JZSynthesizerXg* IsXG() const
    {
      return this;
    }

    virtual JZEvent* CreateResetEvent()
    {
      return Sysex(0, SX_XG_ON);
    }

    virtual JZEvent* ModSX(int index, long clk, int cha, unsigned char val);
    virtual JZEvent* BendSX(int index, long clk, int cha, unsigned char val);
    virtual JZEvent* CafSX(int index, long clk, int cha, unsigned char val);
    virtual JZEvent* PafSX(int index, long clk, int cha, unsigned char val);
    virtual JZEvent* CC1SX(int index, long clk, int cha, unsigned char val);
    virtual JZEvent* CC2SX(int index, long clk, int cha, unsigned char val);
    virtual JZEvent* RxChannelSX(long clk, int cha, unsigned char val);
    virtual JZEvent* UseForRhythmSX(long clk, int cha, unsigned char val);
    virtual JZEvent* ControllerNumberSX(int ctrlno, long clk, int cha, unsigned char val);
    virtual JZEvent* ReverbMacroSX(long clk, unsigned char val, unsigned char lsb = 0);
    virtual JZEvent* ChorusMacroSX(long clk, unsigned char val, unsigned char lsb = 0);
    virtual JZEvent* EqualizerMacroSX(long clk, unsigned char val);
};
