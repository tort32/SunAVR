#pragma once
#include "stdafx.h"

// Implementation of IR receiver working over NEC Protocol
// http://www.sbprojects.com/knowledge/ir/nec.php

// Implementation used INT1 for capture IR signals
// and Counter0 for signals time measuring.
// INT1 interruption set on rising edge
// so we measure signal time of HI-LO form:
//
//  |<--->|
//  |     |
//  +--+  +--+  HI
//  |  |  |  | 
//  |  |  |  | 
// -+  +--+  +- LO

// Counter prescaler set by 64
// Computing counter ticks according to F_CPU:
// Ticks = Time*F_CPU/64

// For F_CPU = 1MHz:
// '0' signal = 1.12ms = 18 ticks
// '1' signal = 2.24ms = 35 ticks
// Repeat = 9 + 2.25 = 11.25ms = 175 ticks
// Start = 9 + 4.5 = 13.5ms = 211 ticks

// For some reason on my real remote controls
// I got 5ms for Start signal and 2,75ms for Repeat signal
// This is 78 and 43 ticks

// IR class contains only user code public methods
// NOTE: Signal capturing code is hidden in IR.cpp
class IR
{
  friend class IR_SignalCapture;
public:
  typedef void (*CommandCallback)(uint8_t, uint8_t);

  static void init(CommandCallback pf, bool callOnRepeat = false);
private:
  static CommandCallback mpCommandCallback;
  static bool mCallOnRepeat;
};


