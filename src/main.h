/*
  main.h - OPAL FW on PJRC Teensy 4.x board

  Part of OpenGalvo - OPAL Firmware

  Copyright (c) 2020-2021 Daniel Olsson

  OPAL Firmware is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPAL Firmware is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPAL Firmware.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#ifdef __AVR__
#error "Sorry, this only works on 32 bit Teensy boards.  AVR isn't supported."
#endif

#if TEENSYDUINO < 121
#error "Minimum PJRC Teensyduino version 1.21 is required"
#endif

#ifndef MAIN_h
#define MAIN_h


#include "PrintSettings.h"
#include <Arduino.h>
//#include <HardwareSerial.h>
#include <configuration.h>
#include <CircularBuffer.h>
#include <XY2_100.h>
#include <LaserController.h>
#include <CO2Laser.h>
#include "Pins.h"
#include "Helpers.h"
#include "MotionMGR.h"
#include "SerialCMDReader.h"
#include "SDCardReader.h"

#define GALVO_CENTER_DEF 32767

#ifdef INVERSE_X
  static bool AXIS_INVERSE_X = true;
#else
  static bool AXIS_INVERSE_X = false;
#endif

#ifdef INVERSE_Y
  static bool AXIS_INVERSE_Y = true;
#else
  static bool AXIS_INVERSE_Y = false;
#endif


#ifdef MAX_ANGLE_X
  static double X_MAX_ANGLE = MAX_ANGLE_X;
#else
  static double X_MAX_ANGLE = 19.0;
#endif

#ifdef MAX_ANGLE_Y
  static double Y_MAX_ANGLE = MAX_ANGLE_Y;
#else
  static double Y_MAX_ANGLE = 19.0;
#endif

#ifdef MIN_ANGLE_X
  static double X_MIN_ANGLE = MIN_ANGLE_X;
#else
  static double X_MIN_ANGLE = -19.0;
#endif

#ifdef MIN_ANGLE_Y
  static double Y_MIN_ANGLE = MIN_ANGLE_Y;
#else
  static double Y_MIN_ANGLE = -19.0;
#endif

#ifdef GALVO_X_CENTER
  static int GALVO_X_CENTER_VAL = GALVO_X_CENTER;
#else
  static int GALVO_X_CENTER_VAL = GALVO_CENTER_DEF;
#endif

#ifdef GALVO_Y_CENTER
  static int GALVO_Y_CENTER_VAL = GALVO_Y_CENTER;
#else
  static int GALVO_Y_CENTER_VAL = GALVO_CENTER_DEF;
#endif


#ifdef GALVO_X_MIN
  static int GALVO_X_MIN_VAL = GALVO_X_MIN;
#else
  static int GALVO_X_MIN_VAL = 0;
#endif

#ifdef GALVO_Y_MIN
  static int GALVO_Y_MIN_VAL = GALVO_Y_MIN;
#else
  static int GALVO_Y_MIN_VAL = 0;
#endif

#ifdef GALVO_X_MAX
  static int GALVO_X_MAX_VAL = GALVO_X_MAX;
#else
  static int GALVO_X_MAX_VAL = 65535;
#endif

#ifdef GALVO_Y_MAX
  static int GALVO_Y_MAX_VAL = GALVO_Y_MAX;
#else
  static int GALVO_Y_MAX_VAL = 65535;
#endif




void process();
void setLaserPower(double PWM); 
void setGalvoPosition(double x, double y);
bool ReadSerial5();
void xinit_process_string(char instruction[]);
void setNextFWDMSG(char MSG[150]);

#endif