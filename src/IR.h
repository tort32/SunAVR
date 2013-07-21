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


// Computing counter ticks according to F_CPU:
// Ticks = Time*F_CPU/TimerPrescale

// For F_CPU = 1MHz and Presacale = 64:
// '0' signal = 1.12ms = 18 ticks
// '1' signal = 2.24ms = 35 ticks
// Repeat = 9 + 2.25 = 11.25ms = 175 ticks
// Start = 9 + 4.5 = 13.5ms = 211 ticks

// For some reason on my real remote controls
// I got 5ms for Start signal and 2,75ms for Repeat signal
// This is 78 and 43 ticks

// For F_CPU = 8MHz and Presacale = 256:
// '0' signal = 1.12ms = 35 ticks
// '1' signal = 2.24ms = 70 ticks
// Repeat = 2.75ms = 86 ticks
// Start = 5.00ms = 156 ticks

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


