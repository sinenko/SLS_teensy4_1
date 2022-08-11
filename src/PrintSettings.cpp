#include "PrintSettings.h"

// static PrintSettings::CurParams currentParams;

int PrintSettings::speed = 100;
char PrintSettings::currentPrintingFile[255];
char PrintSettings::currentCopyingFile[255];
bool PrintSettings::isPrinting = false;
bool PrintSettings::isCopying = false;

PrintSettings::PrintSettings()
{

}
