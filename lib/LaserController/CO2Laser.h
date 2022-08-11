

#pragma once

#ifndef CO2LASER_h
#define CO2LASER_h

#include <Arduino.h>
#include <string.h>
#include "LaserController.h"

#define LASER_RESOLUTION 12

class CO2Laser : public LaserController {
	public:
		CO2Laser();
    void begin(int PWM_OUT_Pin, int ENABLE_Pin);
    void stop(void);
    bool isInitiallized();
		void handleLaser();
		void update(uint16_t PWM);
		void update();
    void on();
    void off();
   
	private:
    uint16_t                laserPWM_OUT_Pin; // = LASER_OUT_PIN;
    uint16_t                laserEn_Pin; // = 10;
    
    const static uint16_t   ticklePWM           = 4;
    int                     laserPWM            = 0;
    int                     laserPWMLowerLimit  = 50;
    uint32_t                tickleStart         = 0x0;
    uint32_t                laserInitTime       = {0x1388}; //Millis to wait in warmup state / Tickle state during init
    int                     currentFreq         = 0;
    int                  oldlaserPWM         = 0;

void set20kPWM(int PWM) {
  if(currentFreq != 20000) {
    currentFreq = 20000;
    analogWriteFrequency(laserPWM_OUT_Pin, 20000);
  }
  analogWrite(laserPWM_OUT_Pin, laserPWM); //Output Laser
}

void set5kPWM() {
  if(currentFreq != 5000) {
    currentFreq = 5000;
    analogWriteFrequency(laserPWM_OUT_Pin, 5000);
  }
  analogWrite(laserPWM_OUT_Pin, CO2Laser::ticklePWM); //Output Trickle
}

};
#endif
