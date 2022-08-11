#include "SerialCMDReader.h"


SerialCMDReader::SerialCMDReader(CircularBuffer<GCode, BUFFERSIZE> *buf, SDCardReader *sdreader)
{
  bufRef = buf;
  _sdreaderRef = sdreader;
}

void SerialCMDReader::begin(){}

void SerialCMDReader::stop(){}

void SerialCMDReader::handleSerial(){

  if (!bufRef->isFull()){
    if(Serial){
      static char worda[COMMAND_SIZE], *pSdata=worda;
      byte ch;
      // Чтение данных из Serial порта
      if(Serial.available()){
        ch = Serial.read();
        cnt++;
        if ((pSdata - worda)>=COMMAND_SIZE-1) {
          pSdata--;
          Serial.print("BUFFER OVERRUN\n");
        }
        *pSdata++ = (char)ch;
      }

      if(ch == '\n'){
        pSdata = worda;
        if (worda[0] == '/' || worda[0] == '(' || worda[0] == ';'){
          return;
        }
          
        if (PrintSettings::isCopying){
          GCode* tmp = SerialCMDReader::parseCopyingString(worda);
          // Отправка файла с OctoPrint на флешку
          if(tmp->code == 29 && tmp->codeprefix == 'M') {
              PrintSettings::isCopying = false;
              _sdreaderRef->closeFile();
          } else {
            _sdreaderRef->streamWriteFileLine(tmp->customCommand);
          }
          Serial.println("ok");
          delete tmp->customCommand;
          delete tmp;

        } else {
          // Отправка команд с OctoPrint в буфер для обработки
          GCode* tmp = SerialCMDReader::process_string(worda);
          SerialCMDReader::bufRef->unshift(*tmp);
          delete tmp;
        }
        
        init_process_string(worda);
      }
    }

    // Печать через SD карту
    if(PrintSettings::isPrinting){
      static char wordaSD[COMMAND_SIZE];
      _sdreaderRef->streamReadFileLine(wordaSD);
      GCode* tmp = SerialCMDReader::process_string(wordaSD);
      SerialCMDReader::bufRef->unshift(*tmp);
      delete tmp;
      init_process_string(wordaSD);
    }
  }

}

int SerialCMDReader::searchLetter(char *word, char letter)
{
  int pointer = 0;
  while (word[pointer] != '\0' || word[pointer] !='\n')
  {
    if(word[pointer] == letter) return pointer;
    pointer++;
  }
  return -1;
}

GCode* SerialCMDReader::parseCopyingString(char instruction[])
{
  GCode* newCode = new GCode();

  byte stringLen = strlen(instruction);
  //instruction[stringLen + 1] = '\0'; // maybe don't work

  int posCheckSumPointer = searchLetter(instruction, '*');

  if(posCheckSumPointer != -1)
  {
    char inputChecksum[stringLen - posCheckSumPointer + 1];
    for(int i = posCheckSumPointer + 1, j = 0; i <= stringLen; i++, j++)
    {
      inputChecksum[j] = instruction[i];
    }
     // Подсчёт чексуммы строки
    int checksum = 0;
    for(int i = 0; instruction[i] != '*' && instruction[i] != '\0'; i++)
    {
      checksum ^= instruction[i];
    }
    checksum &= 0xff;

    // Если чексумма верна
    // if((double)strtod(inputChecksum, NULL) == (double)checksum) Serial.println("Uwu");
    
    if((double)strtod(inputChecksum, NULL) != (double)checksum)
    {
      Serial.println("ok");
      Serial.println("Bad checksum, stopping copy");
      PrintSettings::isCopying = false;
      return newCode;
    }
  } 
  else 
  {
      Serial.println("ok");
      Serial.println("Bad string, no checksum, stopping copy");
      PrintSettings::isCopying = false;
      return newCode;  
  }

  // int posFirstSpace = searchLetter(instruction, ' ');
  // char stringWithoutN[posCheckSumPointer - posFirstSpace + 1];
  // for(int i = posFirstSpace + 1, j = 0; i != posCheckSumPointer; i++, j++)
  // {
  //   stringWithoutN[j] = instruction[i];
  // }
  // stringLen = strlen(stringWithoutN) + 2;

  char lastComand = (char)0;
  char temp[stringLen] = ""; 
  int k_instr = 0; 

  for (byte i=0; i<stringLen; i++){
    switch(instruction[i]){
      
      case 'M':
      case 'N':
        lastComand = instruction[i];
        for(byte k=0; k<stringLen; k++){
          temp[k] = '\0';
        }
        k_instr = 0;
        continue;
        break;
      case ' ':
      case '\n':
      case '*':
        if(lastComand == 'M'){
            newCode->codeprefix = 'M';
            newCode->code = (double)strtod(temp, NULL);
            if(newCode->code == 29) {
              return newCode;
            }
        }
        if(lastComand == 'N') {
          lastComand = (char)0;
          k_instr = 0;
          for(byte k=0; k<stringLen; k++){
            temp[k] = '\0';
          }
        }
        if(instruction[i] == '*'){
          goto endOfCycle;
        }
        temp[k_instr] = instruction[i];
        k_instr++;
        break;
      default:
        // if(instruction[i] != ' ' && instruction[i] != 13){
          temp[k_instr] = instruction[i];
          k_instr++;
        // }
        break;
    }
  }
  
  endOfCycle:;
  temp[k_instr] = '\n';
  newCode->customCommand = new char[stringLen];
  strcpy(newCode->customCommand, temp);   

  return newCode;
}

GCode* SerialCMDReader::process_string(char instruction[])
{
  //process our command!
  
  GCode* newCode = new GCode();
  newCode->x = MAX_VAL;
  newCode->y = MAX_VAL;
  newCode->z = MAX_VAL;
  newCode->e = MAX_VAL;
  newCode->f = MAX_VAL;
  newCode->s = MAX_VAL;
  
  newCode->i = MAX_VAL;
  newCode->j = MAX_VAL;
  //TODO: determine if delete newcode is needed to keep memmory clean...
  
  byte intstructionLen = strlen(instruction);
  char lastComand = (char)0;
  char temp[intstructionLen] = ""; //sizeof(in)
  int k_instr = 0; // Размер текущего положения символа в строке
  
  for (byte i=0; i<intstructionLen; i++){
    switch(instruction[i]){
      case 'G':
      case 'M':
      case 'X':
      case 'Y':
      case 'Z':
      case 'E':
      case 'F':
      case 'S':
        lastComand = instruction[i];
        for(byte k=0; k<intstructionLen; k++){
          temp[k] = '\0';
        }
        k_instr = 0;
        continue;
        break;
      case ' ':
      case '\n':
        switch(lastComand){
          case 'G':
            newCode->codeprefix = 'G';
            newCode->code = (double)strtod(temp, NULL);
            break;
          case 'M':
            newCode->codeprefix = 'M';
            newCode->code = (double)strtod(temp, NULL);

            switch(newCode->code){
              case 23:
              case 28:
              case 30:
                goto endOfCycle;
                break;
              case 29:
                return newCode;
                break;
            }
            break;
          case 'X':
            newCode->x = (double)strtod(temp, NULL);
            break;
          case 'Y':
            newCode->y = (double)strtod(temp, NULL);
            break;
          case 'Z':
            newCode->z = (double)strtod(temp, NULL);
            break;
          case 'E':
            newCode->e = (double)strtod(temp, NULL);
            break;
          case 'F':
            newCode->f = (double)strtod(temp, NULL);
            break;
          case 'S':
            newCode->s = (double)strtod(temp, NULL);
            break;
        }
        continue;
        break;
      default:
        if(instruction[i] != ' ' && instruction[i] != 13){
          temp[k_instr] = instruction[i];
          k_instr++;
        }
        break;
    }
  }
  return newCode;

  endOfCycle:;

  if(newCode->codeprefix == 'M'){
    k_instr = 0;
    switch(newCode->code){
      case 23:
      case 28:
      case 30:
        for(byte i=4; i<intstructionLen; i++){
          if(instruction[i] != ' ' && instruction[i] != 13 && instruction[i] != 10){
            temp[k_instr] = instruction[i];
            k_instr++;
          }
        }
        
        newCode->customCommand = new char[intstructionLen];
        strcpy(newCode->customCommand, temp);   
        Serial.println(newCode->customCommand); //M23 test2.gcode
        break;
    }
  }

  return newCode;
}
