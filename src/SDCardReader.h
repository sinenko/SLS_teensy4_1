#pragma once

#ifdef __AVR__
#error "Sorry, this only works on 32 bit Teensy boards.  AVR isn't supported."
#endif

#if TEENSYDUINO < 121
#error "Minimum PJRC Teensyduino version 1.21 is required"
#endif

#include <Arduino.h>
#include "helpers.h"
#include "SD.h"
#include "SPI.h"
#include "Pins.h"
#include "string.h"



struct Files {
  bool isProcessCatalogReading = false;
  File currentOpenningFile;
  File currentOppeningDir;
};


class SDCardReader
{
    public:
	    SDCardReader();
        void init();
        void openFileRead(char* path); // path - разделитель пути '/'

        void openWriteFile(const char * const path);
        void streamReadFileByte(int16_t* newByte);
        void streamReadFileLine(char* line);

        void openFileWriteBegin(char* path); // path - разделитель пути '/'
        void createFileAndOpenToWrite(char* path); // path - разделитель пути '/'
        void streamWriteFileLine(char* dataString);

        void deleteFileOnSD(char* path);

        void closeFile();
        bool printListOfFiles(char* filePath);
        Files files;
        bool SDInited = false;
    private:
        void resetCharCounters();
        void scanFilesInDir(File dir, char* result);
        bool fileOpened = false;
        int16_t byteRead = 0;
        byte cursorPosLine = 0;
};
