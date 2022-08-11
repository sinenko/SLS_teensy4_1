#include "CO2Laser.h"


CO2Laser::CO2Laser()
{
}

void CO2Laser::update(uint16_t PWM)
{
  laserPWM = PWM;
  handleLaser();
}

void CO2Laser::update()
{
  handleLaser();
}

void CO2Laser::begin(int PWM_OUT_Pin, int ENABLE_Pin)
{
  laserPWM_OUT_Pin = PWM_OUT_Pin;
  laserEn_Pin = ENABLE_Pin;
  
  pinMode(laserEn_Pin, OUTPUT);
  digitalWrite(ENABLE_Pin, 1);

  analogWriteResolution(LASER_RESOLUTION);
  pinMode(laserPWM_OUT_Pin, OUTPUT);
  
  laserPWM = 0;
  oldlaserPWM = 0;
}

void CO2Laser::stop()
{
  laserPWM = 0;
  digitalWrite(laserEn_Pin, 1);
  analogWrite(laserPWM_OUT_Pin, 0);
}

void CO2Laser::on()
{
  digitalWrite(laserEn_Pin, 0);
}

void CO2Laser::off()
{
  digitalWrite(laserEn_Pin, 1);
}

void CO2Laser::handleLaser() //M23 test2.gcode
{
  if( laserPWM==oldlaserPWM) //Nothing changed
    return;
  if(laserPWM!=oldlaserPWM)
    oldlaserPWM = laserPWM;
  
  this->set20kPWM(laserPWM);
}

