#pragma once
#ifndef CONFIGURATION_H

//#define CMDBUFFERSIZE 100 //Number of cashed GCodes
//#define MBUFFERSIZE 20  //Buffersize for MCODES - number of consecutive M-Codes before another G-Code
#define DEFAULT_FEEDRATE 100

//#define LASER_IS_SYNRAD
#define LASER_RESOLUTION 12
//#define LASER_PWM_MAX 50
//#define LASER_G0_OFF_G1_ON


#define GALVO_X_CENTER ((36352+256))
#define GALVO_Y_CENTER ((33536+256))

//Растояние между осями зеркал
#define XY_SHIFTS_LEN 13.5

//Растояние от Y зеркала (которое дальше от лазера) до края корпуса сканатор (при услови что зеркало в центре)
#define Y_RAY_FROM_SHIFT 22.4

//Растояние от края сканатора до стола (от сканатора до ДСП + толщина ДСП + от ДСП до стола)
#define Y_RAY_FROM_BODY ((48.24 + 16.3 + 316.5))

//Растояние от Y зеркала (которое дальше от лазера) до стола (при услови что зеркало в центре)
#define Y_RAY_LENGTH ((Y_RAY_FROM_SHIFT+Y_RAY_FROM_BODY))  //403.44

//Размеры получившизся сторон после центровки зеркал
#define LEN_X_DEFAULT 265 
#define LEN_Y_DEFAULT 275

//Область печати
#define PRINT_AREA_X 210
#define PRINT_AREA_Y 210

#define LEN_X_MM 265 //265.0
#define LEN_Y_MM 275 //275.0 

#define MAX_ANGLE_X (atan((LEN_X_MM/2.0)/Y_RAY_LENGTH)) //18.182 C
#define MAX_ANGLE_Y (atan((LEN_Y_MM/2.0)/Y_RAY_LENGTH)) //18.82 C
#define MIN_ANGLE_X ((-1.0)*MAX_ANGLE_X) //-18.182 C
#define MIN_ANGLE_Y ((-1.0)*MAX_ANGLE_Y) //-18.82 C




#define X_MIN 0 //mm
#define Y_MIN 0 //mm

#define X_MAX LEN_X_MM //mm
#define Y_MAX LEN_Y_MM //mm

//#define INVERSE_X
#define INVERSE_Y

#define CONFIGURATION_H
#endif