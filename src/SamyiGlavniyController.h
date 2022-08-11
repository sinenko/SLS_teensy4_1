#pragma once

#include "SDCardReader.h"
#include "SerialCMDReader.h"

class SamyiGlavniyController
{
    public:
      SamyiGlavniyController();
      SDCardReader sdReader;
      SerialCMDReader *serialReader;
    private:
};

