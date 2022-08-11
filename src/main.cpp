/*
  main.cpp - Main projectfile to run OPAL FW on PJRC Teensy 4.x board

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

#include "main.h"


static CircularBuffer<GCode, BUFFERSIZE> commandBuffer;
static CircularBuffer<String, BUFFERSIZE> FWDBuffer;
static SDCardReader sdreader;

MotionMGR* motion;
XY2_100* galvo;
SerialCMDReader *serialReciever;

// PrintSettings *printSettings;


LaserController *laser;

void setup() {
  
  //******* корректировка зеркал по центру  НАЧАЛО **/
  int OFFSET_CENTER_X = (abs(GALVO_X_CENTER_VAL-GALVO_CENTER_DEF)*2);
  int OFFSET_CENTER_Y = (abs(GALVO_Y_CENTER_VAL-GALVO_CENTER_DEF)*2);

  if(GALVO_X_CENTER_VAL>GALVO_CENTER_DEF){
    if(AXIS_INVERSE_X)
     GALVO_X_MAX_VAL -= OFFSET_CENTER_X;
    else
      GALVO_X_MIN_VAL += OFFSET_CENTER_X;

    if(AXIS_INVERSE_Y)
      GALVO_Y_MAX_VAL -= OFFSET_CENTER_Y;
    else
      GALVO_Y_MIN_VAL += OFFSET_CENTER_Y;
  } else {
    if(AXIS_INVERSE_X)
      GALVO_X_MIN_VAL += OFFSET_CENTER_X;
    else
      GALVO_X_MAX_VAL -= OFFSET_CENTER_X;

    if(AXIS_INVERSE_Y)
      GALVO_Y_MIN_VAL += OFFSET_CENTER_Y;
    else
      GALVO_Y_MAX_VAL -= OFFSET_CENTER_Y;
  }

  //******* корректировка зеркал по центру  КОНЕЦ **/
  

  //******* корректировка и обрезка размеров области печати НАЧАЛО **/


  //Текущие размеры
  double y_cord_current = 194.5;
  double x_cord_current = 217;

  // //Целевые размеры
  double x_cord_target = PRINT_AREA_X - 11; //В итоге получилось 210
  double y_cord_target = PRINT_AREA_Y - 10.75; //В итоге получилось 210

  double y_K = abs(atan(y_cord_target/2.0/Y_RAY_LENGTH) / atan(y_cord_current/2.0/Y_RAY_LENGTH)); //Вычисляем коэффициент отношения углов поворота зеркал
  double GALVO_LENGTH_Y = GALVO_Y_MAX_VAL - GALVO_Y_MIN_VAL;  //Вычисляем длину хода зеркала Y в Тугриках (0-65 535)
  double GALVO_LENGTH_NEW_Y = GALVO_LENGTH_Y * y_K; //Корректируем диапазон хода зеркал на основании полученного коэффициента
  GALVO_Y_MIN_VAL += (GALVO_LENGTH_NEW_Y-GALVO_LENGTH_Y)/2.0; //Получаем разницу в диапазонах хода зеркал и эту разницу обрезаем (прибавляем если отрицательна) с обеих сторон
  GALVO_Y_MAX_VAL -= (GALVO_LENGTH_NEW_Y-GALVO_LENGTH_Y)/2.0; //Получаем разницу в диапазонах хода зеркал и эту разницу обрезаем (прибавляем если отрицательна) с обеих сторон

  double x_K = abs(atan(x_cord_target/2.0/Y_RAY_LENGTH) / atan(x_cord_current/2.0/Y_RAY_LENGTH));  //Вычисляем коэффициент отношения углов поворота зеркал
  double GALVO_LENGTH_X = GALVO_X_MAX_VAL - GALVO_X_MIN_VAL;  //Вычисляем длину хода зеркала Y в Тугриках (0-65 535)
  double GALVO_LENGTH_NEW_X = GALVO_LENGTH_X * x_K; //Корректируем диапазон хода зеркал на основании полученного коэффициента
  GALVO_X_MIN_VAL += (GALVO_LENGTH_NEW_X-GALVO_LENGTH_X)/2.0; //Получаем разницу в диапазонах хода зеркал и эту разницу обрезаем (прибавляем если отрицательна) с обеих сторон
  GALVO_X_MAX_VAL -= (GALVO_LENGTH_NEW_X-GALVO_LENGTH_X)/2.0; //Получаем разницу в диапазонах хода зеркал и эту разницу обрезаем (прибавляем если отрицательна) с обеих сторон

  
  //******* корректировка и обрезка размеров области печати КОНЕЦ **/

  sdreader.init();

  serialReciever = new SerialCMDReader(&commandBuffer, &sdreader);
  serialReciever->begin();

  //init Galvo Protocol
  galvo = new XY2_100();
  galvo->begin(); //TODO:ADD define "Galvo has SSR" for galvo PSU

  laser = new CO2Laser();
  laser->begin(LASER_PWM_OUT_PIN, LASER_ENABLE_PIN);

  

  motion = new MotionMGR(&commandBuffer);
  motion->begin(galvo, laser, &sdreader);

  Serial5.begin(115200);
  Serial5.print("G28\n");

  while (!Serial);
  Serial.println("ok");

}


char* nextFWDMSG[150];

void loop() { 
  serialReciever->handleSerial();
  motion->tic();
}

void setGalvoPosition(double x, double y)
{
  int tmp_x, tmp_y;

  double y_cord = (y-LEN_Y_MM/2.0);
  double x_cord = (x-LEN_X_MM/2.0);


  double x_angle_correction_rad = atan(x_cord / (sqrt(pow(Y_RAY_LENGTH,2)+pow(y_cord,2))+XY_SHIFTS_LEN));
  double y_angle_correction_rad = atan(y_cord / (sqrt(pow(Y_RAY_LENGTH,2)-pow(x_cord,2)/5)));


  if(AXIS_INVERSE_X)
    tmp_x = map(x_angle_correction_rad, X_MIN_ANGLE, X_MAX_ANGLE, GALVO_X_MAX_VAL, GALVO_X_MIN_VAL)+0.5;
  else
    tmp_x = map(x_angle_correction_rad, X_MIN_ANGLE, X_MAX_ANGLE, GALVO_X_MIN_VAL, GALVO_X_MAX_VAL)+0.5;

  if(AXIS_INVERSE_Y)
    tmp_y = map(y_angle_correction_rad , Y_MIN_ANGLE, Y_MAX_ANGLE, GALVO_Y_MAX_VAL, GALVO_Y_MIN_VAL)+0.5;
  else
    tmp_y = map(y_angle_correction_rad , Y_MIN_ANGLE, Y_MAX_ANGLE, GALVO_Y_MIN_VAL, GALVO_Y_MAX_VAL)+0.5;


  // if(AXIS_INVERSE_X)
  //   tmp_x = map(x, 0, 215, GALVO_X_MAX_VAL, GALVO_X_MIN_VAL)+0.5;
  // else
  //   tmp_x = map(x, 0, 215, GALVO_X_MIN_VAL, GALVO_X_MAX_VAL)+0.5;

  // if(AXIS_INVERSE_Y)
  //   tmp_y = map(y , 0, 215, GALVO_Y_MAX_VAL, GALVO_Y_MIN_VAL)+0.5;
  // else
  //   tmp_y = map(y , 0, 215, GALVO_Y_MIN_VAL, GALVO_Y_MAX_VAL)+0.5;   

  // if(AXIS_INVERSE_X)
  //   tmp_x = map(x, 0, 1, 65535, 0)+0.5;
  // else
  //   tmp_x = map(x, 0, 1, 0, 65535)+0.5;

  // if(!AXIS_INVERSE_Y)
  //   tmp_y = map(y , 0, 1, 65535, 0)+0.5;
  // else
  //   tmp_y = map(y , 0, 1, 0, 65535)+0.5;

  // if (tmp_x % 100 == 0){
  //     Serial.print(" X angle="); Serial.print(x_angle); Serial.print(" X="); Serial.print(x);  Serial.print(" x_cor_angle="); Serial.print(x_cor_angle); Serial.print(" X_MIN_ANGLE="); Serial.print(X_MIN_ANGLE);  Serial.print(" X_MAX_ANGLE="); Serial.print(X_MAX_ANGLE);  Serial.print(" GALVO_X_MIN_VAL="); Serial.print(GALVO_X_MIN_VAL); Serial.print(" GALVO_X_MAX_VAL="); Serial.println(GALVO_X_MAX_VAL);
  //     Serial.print(" Y angle="); Serial.print(y_angle); Serial.print(" Y="); Serial.print(y);  Serial.print(" y_cor_angle="); Serial.print(y_cor_angle); Serial.print(" Y_MIN_ANGLE="); Serial.print(Y_MIN_ANGLE);  Serial.print(" YX_MAX_ANGLE="); Serial.print(Y_MAX_ANGLE);  Serial.print(" GALVO_Y_MIN_VAL="); Serial.print(GALVO_Y_MIN_VAL); Serial.print(" GALVO_Y_MAX_VAL="); Serial.println(GALVO_Y_MAX_VAL);
  // }
  
  galvo->setPos(tmp_x, tmp_y);
}

void setLaserPower(double PWM)
{
  double tmp_PWMMin = LASER_MIN_PWM_PERCENT;
  double tmp_Max = LASER_MAX;
  int tmp_LaserRes = LASER_RESOLUTION;
  
  double pinVal = map(PWM,0.0,tmp_Max,tmp_PWMMin,(exp2(tmp_LaserRes))+0.0);
  
  laser->update((int)pinVal);
}

void setNextFWDMSG(char MSG[150])
{
  String str = String(MSG);
  FWDBuffer.unshift(str);
}

bool ReadSerial5()
{
  bool retval = false;
  if (Serial5.available()) {
    retval = true;
    static char worda[COMMAND_SIZE], *pSdata=worda;
    byte ch;

    ch = Serial5.read();
    //mcnt++;
    // -1 for null terminator space
    if ((pSdata - worda)>=COMMAND_SIZE-1) {
        pSdata--;
        Serial.print("Serial5: BUFFER OVERRUN\n");
    }
  
    *pSdata++ = (char)ch;
    if (ch=='\n' || ((pSdata - worda)>=COMMAND_SIZE-3))// Command received and ready.
    {
      
      pSdata = worda;
      Serial.print("\nECHO Serial5: ");Serial.println(worda);
      xinit_process_string(worda);
    }
  }
  return retval;
}

/*
Used by Serial5 to clear input string array.
*/
void xinit_process_string(char instruction[])  {
  //init our command
  for (byte i=0; i<COMMAND_SIZE; i++)
    instruction[i] = 0;
  //mcnt = 0;
}