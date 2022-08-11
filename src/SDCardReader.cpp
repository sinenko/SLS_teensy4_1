#include <SDCardReader.h>
#include <PrintSettings.h>


SDCardReader::SDCardReader() {}


void SDCardReader::init()
{
    if (SD.begin(SD_SELECT_PIN)) {
        SDInited = true;
        delay(20);
        Serial.println("SD card ok");
        Serial.println("ok");
        return;
    }

    SDInited = false;
    Serial.println("No SD card");
    Serial.println("ok");
}

void SDCardReader::openFileRead(char* path)
{
    if (SDInited){
        //if(SD.exists(path)){
            files.currentOpenningFile = SD.open(path); //M23 /test.gcode
            fileOpened = true;
            //}
        //} else {
            // На SD нету такого файла
        //}
    } else {
        // SD не подкючена
    } 
}

void SDCardReader::openFileWriteBegin(char* path)
{
    if (SDInited){
            files.currentOpenningFile = SD.open(path, FILE_WRITE); //M23 /test.gcode
            fileOpened = true;
    } else {
        // SD не подкючена
    } 
}

void SDCardReader::createFileAndOpenToWrite(char* path)
{
    if(SD.exists(path)){
        SD.remove(path);
        openFileWriteBegin(path);
        
    } else {
        openFileWriteBegin(path);
        
    }
    //Serial.print("Now doing file: "); Serial.println(PrintSettings::currentCopyingFile);
    Serial.print("Now fresh file: "); Serial.println(PrintSettings::currentCopyingFile);
    Serial.print("Writing to file: "); Serial.println(PrintSettings::currentCopyingFile);
}

void SDCardReader::streamWriteFileLine(char* dataString){
    if (PrintSettings::isCopying && fileOpened){
        files.currentOpenningFile.write(dataString);
    }
}

void SDCardReader::deleteFileOnSD(char* path){
     if(SD.exists(path)){
        SD.remove(path);
     }
}

void SDCardReader::closeFile()
{
    if (fileOpened){
        files.currentOpenningFile.close();
        PrintSettings::isPrinting = false;
        fileOpened = false;
    }
}

void SDCardReader::streamReadFileByte(int16_t* newByte)
{ 
    if (fileOpened){
        *newByte = files.currentOpenningFile.read();
    }
}

void SDCardReader::resetCharCounters()
{
    byteRead = 0;
    cursorPosLine = 0;
}

void SDCardReader::streamReadFileLine(char* line)
{
    do {
        streamReadFileByte(&byteRead);
        if (byteRead == -1) { closeFile(); break; }
        line[cursorPosLine] = (char)byteRead;
        cursorPosLine++;
    }
    while(byteRead != '\n');

    resetCharCounters();
}

void SDCardReader::scanFilesInDir(File dir, char* result)
{
    result[0] = '\0';
    File entry = dir.openNextFile();
    if(!entry){
        return;
    }

    char sizeBuffer[12];
    

    if(entry.isDirectory()){
        strcat(result, "DIR_ENTER: /");
        strcat(result, entry.name());
        strcat(result, "/ ");
        strcat(result, entry.name());
        strcat(result, "\nDIR_EXIT");
    } else {
        strcat(result,entry.name());
        itoa(entry.size(), sizeBuffer, 10);
        strcat(result,  " ");
        strcat(result,  sizeBuffer);
    }
}

//Считываем список файлов в директории, 1 файл за одно считывания. Если список файлов окончен возвращаем false
bool SDCardReader::printListOfFiles(char* filePath)
{
    char result[MAX_FILEPATH_LENGTH];
    if(!files.isProcessCatalogReading){
        files.isProcessCatalogReading = true;
        closeFile();
        files.currentOppeningDir = SD.open(filePath);
        Serial.println("Begin file list:");
        scanFilesInDir(files.currentOppeningDir, result);
        Serial.println(result);
        return true;
    } else {
        scanFilesInDir(files.currentOppeningDir, result);
        if(result[0] == '\0'){
            files.currentOppeningDir.close();
            files.isProcessCatalogReading = false;
            Serial.println("End file list");
            Serial.println("ok");
            return false;
        } else {
            Serial.println(result);
            return true;
        }
        
    } 
}

 