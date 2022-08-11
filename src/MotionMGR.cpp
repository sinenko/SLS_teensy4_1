#include "MotionMGR.h"

XY2_100* _galvo;
LaserController* _laser;
GCode* currentGcode;


MotionMGR::MotionMGR(CircularBuffer<GCode, BUFFERSIZE> *buf)
{
  bufRef = buf;
}

void MotionMGR::begin(XY2_100* galvo, LaserController* laser, SDCardReader* sdReader)
{
  _galvo = galvo;
  _laser = laser;
  _status = IDLE;
  _sdreader = sdReader;
}

MotionStatus MotionMGR::getStatus()
{
  return _status;
}

void MotionMGR::tic()
{
  _NOW = nanos();
  
  switch (_status) {
    case IDLE: processGcodes(); break;
    case INTERPOLATING: interpolateMove(); break;
    default: break;
  }
  setGalvoPosition(CURRENT_CMD_X, CURRENT_CMD_Y);

  if (readingListFilesOnSD){
    if(!_sdreader->printListOfFiles("/")) readingListFilesOnSD = false;
  }

  if(CURRENT_LASERENABLED)
  {
    if(LASER_CHANGED)
    {
      setLaserPower(CURRENT_S);
      LASER_CHANGED = false;
    }
  }
  else
    setLaserPower(0);
}


void MotionMGR::processMcode(GCode* code)
{
  if(code->codeprefix == 'M') {
    //Handle MCodes
    switch (code->code)
    {
      case 3:
      case 4:
      //M3 / M4
        setVal(&CURRENT_S, code->s);
        CURRENT_LASERENABLED = true;
        LASER_CHANGED = true;
        break;
      case 5:
      //M5
        CURRENT_LASERENABLED = false;
        break;
        
      case 17:
      //M17
        digitalWrite(GALVO_SSR_OUT_PIN,1);
        return;
      case 18:
      //M18
        digitalWrite(GALVO_SSR_OUT_PIN,0);
        return;

      case 80:
      //M80
        _laser->on();
        return;
      case 81:
      //M81
        _laser->off();
        return;
      case 110:
        Serial.println("ok");
        break;
      case 115:
        Serial.println("ok PROTOCOL_VERSION:0.1 FIRMWARE_NAME:FiveD MACHINE_TYPE:Mendel EXTRUDER_COUNT:1");
        break;
      case 105:
        Serial.println("ok T:777 C:666");
        break;
      case 20:
        readingListFilesOnSD = true;
        PrintSettings::isCopying = false;
        break;
      case 21:
        _sdreader->init();
        break;
      case 22:
        _sdreader->SDInited = false;
        Serial.println("ok");
        break;      
      case 23:
        strcpy(PrintSettings::currentPrintingFile, currentGcode->customCommand); 

        if(!PrintSettings::isPrinting){
          PrintSettings::isPrinting = true;
          _sdreader->openFileRead(PrintSettings::currentPrintingFile);
          Serial.println("ok");
        }
        break;     
      case 28:
        strcpy(PrintSettings::currentCopyingFile, currentGcode->customCommand); 
        if(!PrintSettings::isCopying){
          PrintSettings::isCopying = true;
          _sdreader->createFileAndOpenToWrite(PrintSettings::currentCopyingFile);
          Serial.println("ok");
        }
        break;
      case 29:
        PrintSettings::isCopying = false;
        _sdreader->closeFile();
        Serial.println("ok");
        break;      
      case 30:
        _sdreader->deleteFileOnSD(currentGcode->customCommand);
        Serial.println("ok");
        break;
      default:
        Serial.println("ok");
        break;
    }
  }
  else  {
    //Handle GCodes
    switch (code->code)
    {
    case 90:
    //G90
      CURRENT_ABSOLUTE = true;
      break;
    case 91:
    //G91
      CURRENT_ABSOLUTE = false;
      break;    
    default:
      break;
    }
  }
}


bool MotionMGR::processGcodes()
{
  bool gcodeFound = false;
  if(!bufRef->isEmpty())
  { 
    currentGcode = new GCode((bufRef->pop()));
    // Упростить условие можно
    if(!( 
      (*currentGcode).codeprefix == 'G' && (
            (*currentGcode).code == 0 ||
            (*currentGcode).code == 1 ||
            (*currentGcode).code == 2 ||
            (*currentGcode).code == 3 ||
            (*currentGcode).code == 28 )))
    {
      processMcode(currentGcode);
    }
    else
    {
      processGcode(currentGcode);
    }
    gcodeFound = true; 

    delete currentGcode;
  }
  return gcodeFound;
}
void MotionMGR::processGcode(GCode* code)
{
  double new_S = 0;
  setVal(&new_S, code->s);
  LASER_CHANGED = new_S != CURRENT_S;

  CURRENT_CODE = 0;
  switch (code->code) {
    case 0:
      CURRENT_CODE = 0;
      setVal(&CURRENT_F, code->f);
      setXY(code);
      break;
   case 1:
      CURRENT_CODE = 1;
      setVal(&CURRENT_F, code->f);
      setVal(&CURRENT_S, code->s);
      setXY(code);
      break;
    case 28:
      CURRENT_CODE = 28;
      CURRENT_TO_X = 0;
      CURRENT_TO_Y = 0;
      CURRENT_TO_Z = 0;
      break;
    default:
      CURRENT_CODE = 9999;
      break;
  }
  _status = INTERPOLATING;
}

void MotionMGR::setVal(double* varToSet, double valToSet)
{
  if(valToSet!=MAX_VAL)
    *varToSet = valToSet;
}
void MotionMGR::setValG91(double* varToSet, double valToSet, double base)
{
  if(valToSet!=MAX_VAL)
    *varToSet = base + valToSet;
  else
    *varToSet = base;
}
void MotionMGR::setXY(GCode* code)
{
  if(CURRENT_ABSOLUTE) {                               //G90 - ABSOLUTE
    setVal(&CURRENT_TO_X, code->x);
    setVal(&CURRENT_TO_Y, code->y);
    setVal(&CURRENT_TO_Z, code->z);
  }
  else{                                               //G91 - RELATIVE
    setValG91(&CURRENT_TO_X, code->x,CURRENT_FROM_X);
    setValG91(&CURRENT_TO_Y, code->y,CURRENT_FROM_Y);
    setValG91(&CURRENT_TO_Z, code->z,CURRENT_FROM_Z);
  }
}

void MotionMGR::interpolateMove()
{
  if(isMoveFirstInterpolation)
  {
    if(CURRENT_CODE == 0 || CURRENT_CODE == 28)
    {
      //dont interpolate
      CURRENT_FROM_X = CURRENT_TO_X;
      CURRENT_FROM_Y = CURRENT_TO_Y;
      CURRENT_FROM_Z = CURRENT_TO_Z;
      CURRENT_CMD_X = CURRENT_TO_X;
      CURRENT_CMD_Y = CURRENT_TO_Y;
      CURRENT_CMD_Z = CURRENT_TO_Z;
      _status = IDLE;
      isMoveFirstInterpolation = true;
      return;
    }
    if(CURRENT_CODE == 1)
    {
      CURRENT_DISTANCE_X = CURRENT_TO_X-CURRENT_FROM_X;
      CURRENT_DISTANCE_Y = CURRENT_TO_Y-CURRENT_FROM_Y;
      calculateMoveLengthNanos(CURRENT_DISTANCE_X, CURRENT_DISTANCE_Y, CURRENT_F, &CURRENT_DURATION);
      CURRENT_STARTNANOS = _NOW;
      CURRENT_ENDNANOS = _NOW + CURRENT_DURATION;
      isMoveFirstInterpolation = false;
    }
  }

  //Actual interpolation
  if(_NOW >= CURRENT_ENDNANOS)
  {
    //done interpolating
    CURRENT_FROM_X = CURRENT_TO_X;
    CURRENT_FROM_Y = CURRENT_TO_Y;
    CURRENT_FROM_Z = CURRENT_TO_Z;
    CURRENT_CMD_X = CURRENT_TO_X;
    CURRENT_CMD_Y = CURRENT_TO_Y;
    CURRENT_CMD_Z = CURRENT_TO_Z;
    _status = IDLE;
    isMoveFirstInterpolation = true;
    return;
  }
  else
  {
    double fraction_of_move = (double)(_NOW-CURRENT_STARTNANOS)/CURRENT_DURATION;
    CURRENT_CMD_X = (CURRENT_FROM_X + (CURRENT_DISTANCE_X*fraction_of_move));
    CURRENT_CMD_Y = (CURRENT_FROM_Y + (CURRENT_DISTANCE_Y*fraction_of_move));
    return;
  }
}

/* 
  MotionMGR::calculateMoveLengthNanos
    Velocity is presumed to be in mm/s
    lengthOfMove = calc hypotenuse a^2+b^2=c^2
    result <-- (mm)/(mm/s) = s   (movelength/moveVolocity) -> (example) 2units / (4units/second) = 0.5seconds *1000 = 500ms
 */
void MotionMGR::calculateMoveLengthNanos(double xdist, double ydist, double moveVelocity, double* result)  {  
  //TODO: Verify unit conversions

  double lengthOfMove = sqrt( (0.0 + xdist)*(0.0 + xdist)  + (0.0 + ydist)*(0.0 + ydist) ); 
  *result = ((lengthOfMove*1000*1000*1000/ (moveVelocity)));  
  return;
}







