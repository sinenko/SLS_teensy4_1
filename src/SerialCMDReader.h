/*
  SerialCMDReader.h - driver code to interpret GCode over Serial on PJRC Teensy 4.x board

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

#ifndef SERIALCMDREADER_H
#define SERIALCMDREADER_H

#include <Arduino.h>
#include "main.h"
#include <CircularBuffer.h>
#include "helpers.h"
#include "SDCardReader.h"
//#include "Ping.h"
#include "PrintSettings.h"
#include <string.h>

#define COMMAND_SIZE 150


class SerialCMDReader
{
	public:
		SerialCMDReader(CircularBuffer<GCode, BUFFERSIZE> *buf, SDCardReader *sdreader);
    void begin();
    void stop(void);
    
    void handleSerial();
    GCode* process_string(char instruction[]);
    GCode* parseCopyingString(char instruction[]);
    
    // void parseCopyingString(char inString[], char* outString, bool* isEndOfFile);

  private:
    int cnt = 0;
    File _gcode_file;
    bool _file_opened = false;
    CircularBuffer<GCode, BUFFERSIZE> *bufRef;
    SDCardReader *_sdreaderRef;
    int searchLetter(char *word, char letter);




    
    double getVal(char chr , char *chArr, int length)
    {
      if(has_command(chr, chArr, length))
        return (double)search_string(chr, chArr, length);
      else
        return MAX_VAL;
    }

    void init_process_string(char instruction[])  {
      //init our command
      for (byte i=0; i<COMMAND_SIZE; i++)
        instruction[i] = 0;
      cnt = 0;
    }
  
    bool has_command(char key, char instruction[], int string_size) {
      for (byte i=0; i<string_size; i++)    {
        if (instruction[i] == key)
          return true;
      }
      return false;
    }

    int has_command_at(char key, char instruction[], int string_size) {
      for (byte i=0; i<string_size; i++)    {
        if (instruction[i] == key)
          return i;
      }
      return -1;
    }
    
    double search_string(char key, char instruction[], int string_size) {
      char temp[10] = "";
      for (byte i=0; i<string_size; i++) {
        if (instruction[i] == key) {
          i++;      
          int k = 0;
          while (i < string_size && k < 10) {
            if (instruction[i] == 0 || instruction[i] == ' '
            || instruction[i] == 'E'
            || instruction[i] == 'F'
            || instruction[i] == 'I'
            || instruction[i] == 'J'
            || instruction[i] == 'P'
            || instruction[i] == 'R'
            || instruction[i] == 'S'
            || instruction[i] == 'T'
            || instruction[i] == 'X'
            || instruction[i] == 'Y'
            || instruction[i] == 'Z'
              ) { break; } 

            temp[k] = instruction[i];
            i++;
            k++;
          }
          return strtod(temp, NULL);
        }
      }
      return 0;
    }
};
#endif
