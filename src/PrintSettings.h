
#pragma once


class PrintSettings
{
public:
  PrintSettings();
  static int speed;
  static char currentPrintingFile[255];
  static char currentCopyingFile[255];
  static bool isPrinting;
  static bool isCopying;
private:

};