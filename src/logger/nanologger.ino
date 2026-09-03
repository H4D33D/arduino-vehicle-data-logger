//P303 Final Project
//Hadeed Fawad
//Mainly for LCD Screen functionaity, and calls datalogger.h for data. 

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <OBD2UART.h>
#include <MicroLCD.h>
#include "images.h"
#include "config.h"
#if USE_SOFTSERIAL
#include <SoftwareSerial.h>
#endif
#include "datalogger.h"


//-----------------------------------
// logger states


#define STATE_SD_READY 0x1
#define STATE_OBD_READY 0x2
#define STATE_GPS_FOUND 0x4
#define STATE_GPS_READY 0x8
#define STATE_ACC_READY 0x10
#define STATE_SLEEPING 0x20


//----------------------------------
//Variable, Values, Arrays Declaration.


static uint32_t lastFileSize = 0;
static int lastSpeed = -1;
static int speed = 0;
static uint16_t fileIndex = 0;
static uint32_t startTime = 0;


static byte pidTier1[]= {PID_RPM, PID_SPEED, PID_ENGINE_LOAD, PID_THROTTLE};
static byte pidTier2[] = {PID_INTAKE_MAP, PID_MAF_FLOW, PID_TIMING_ADVANCE};


#define TIER_NUM1 sizeof(pidTier1)
#define TIER_NUM2 sizeof(pidTier2)


byte pidValue[TIER_NUM1];


//--------------------------------------------------
//Main Class


class COBDLogger : public COBD, public CDataLogger
{


public:
    COBDLogger():state(0) {} //Show Start up screen
    void setup()
    {
        showStates();


        do {
            showStates();
        } while (!init());


        state |= STATE_OBD_READY;


        showStates();


#if ENABLE_DATA_LOG
        uint16_t index = openFile();
        lcd.setFontSize(FONT_SIZE_SMALL);
        lcd.setCursor(86, 0);
        if (index) {
            lcd.write('[');
            lcd.setFlags(FLAG_PAD_ZERO);
            lcd.printInt(index, 5);
            lcd.setFlags(0);
            lcd.write(']');
        } else {
            lcd.print("F1"); //NO LOG - F represents a Failure
            //having these failures identitfiable with integers costs 6 bytes. But saves alot more.
        }
        delay(100);
#endif


#if ENABLE_DATA_LOG
        // open file for logging
        if (!(state & STATE_SD_READY)) {
            if (checkSD()) {
                state |= STATE_SD_READY;
                showStates();
            }
        }
#endif


        initScreen();


    } //end of public: COBDLogger()


    void loop()
    {
        static byte index = 0;
        static byte index2 = 0;
        static byte index3 = 0;


        // poll OBD-II PIDs
        logOBDData(pidTier1[index++]);
        if (index == TIER_NUM1) {
            index = 0;
            if (index2 == TIER_NUM2) {
                index2 = 0;
            } else {
                logOBDData(pidTier2[index2++]);
            }
        }
        //Removed Distance Traveled to Save Bytes (Over 100).


#if USE_MPU6050 //Again this is a gyro sensor, this may be useless.
        if (state & STATE_ACC_READY) {
            processAccelerometer();
        }
#endif


#if ENABLE_DATA_LOG
       
        if (dataSize - lastFileSize >= 1024) { //Creates new save file every 1KB
            flushFile();
            lastFileSize = dataSize;


            // display logged data size


            char buf[7];
            sprintf(buf, "%4uKB", (int)(dataSize >> 10));
            lcd.setFontSize(FONT_SIZE_SMALL);
            lcd.setCursor(92, 7);
            lcd.print(buf);
        }
#endif


        if (errors >= 2) {
            reconnect();
        }
    } //end of loop


//-----------------------------------------------
//SD Card Info and Error Handling


#if ENABLE_DATA_LOG
    bool checkSD()
    {
        Sd2Card card;
        SdVolume volume;
        state &= ~STATE_SD_READY;
        pinMode(SS, OUTPUT);
        if (card.init(SPI_FULL_SPEED, SD_CS_PIN)) {
            const char* type;


            lcd.print("SD"); //Removed Type
            lcd.write(' ');
            if (!volume.init(card)) {
                lcd.print("F2"); //No FAT - F represents failure
                return false;
            }


            uint32_t volumesize = volume.blocksPerCluster();
            volumesize >>= 1; // 512 bytes per block
            volumesize *= volume.clusterCount();
            volumesize >>= 10;


            lcd.print((int)((volumesize + 511) / 1000));
            lcd.print("GB");
        } else {
            lcd.print("SD  ");
            lcd.draw(cross, 16, 16);
            return false;
        }


        if (!SD.begin(SD_CS_PIN)) {
            lcd.print("F3"); //Bad - F represents failure
            return false;
        }


        state |= STATE_SD_READY;
        return true;
    }
#endif


//-----------------------------------------


    void initScreen()
    {
        initLoggerScreen();
    }


//----------------------------------------
//Changed so it would only log Speed and RPM.
//Why is the actual data logger in this program???
//Test to see if this revised code is working.
//Still dont know what this does, since actual datalogging code is in datalogger.h
//However everything is working, and this has been changed from the OG.
//So I really have no idea what its doing.
//I think this just sends the pin info to datalogger.h logData method
private:
    int logOBDData(byte pid)
    {
      int value = 0;
    // send a query to OBD adapter for specified OBD-II pid
    if (readPID(pid, value)) {
        dataTime = millis();
        if (pid == PID_RPM || pid == PID_SPEED) {
            showLoggerData(pid, value);
            // log data to SD card
            logData(0x100 | pid, value);
            //New line can go here, test if all else fails
        }
      }
      return 0; //Indicates Failure
    }


//---------------------------------------------
//Reconnecting Screen


    void reconnect()
    {
#if ENABLE_DATA_LOG
        closeFile();
#endif
        lcd.clear();
        lcd.setFontSize(FONT_SIZE_MEDIUM);
        lcd.print("..."); //Reconnecting - Can leave OBD in while temp away from car.
        startTime = millis();
        state &= ~(STATE_OBD_READY | STATE_ACC_READY);
        state |= STATE_SLEEPING;
        for (uint16_t i = 0; ; i++) {
            if (i == 5) {
                lcd.backlight(false);
                lcd.clear();
            }
            if (init()) {
                int value;
                if (readPID(PID_RPM, value) && value > 0)
                    break;
            }
        }
        state &= ~STATE_SLEEPING;
        fileIndex++;
        setup();
    }


//----------------------------------------------
//Welcome / Start Up Screen
    byte state;


    void showTickCross(bool yes)
    {
        lcd.draw(yes ? tick : cross, 16, 16);
    }
    // screen layout related stuff
    void showStates() //Indicates whether or not vital functions are working.
    {
        lcd.setFontSize(FONT_SIZE_MEDIUM);
        lcd.setCursor(0, 4);
        lcd.print("OBD ");
        showTickCross(state & STATE_OBD_READY);
        lcd.setCursor(0, 6);
        lcd.print("ACC ");
        showTickCross(state & STATE_ACC_READY);
    }


//-------------------------------------------------------------
//Numbers shown when OBD has successfully connected.


    void showLoggerData(byte pid, int value)
    {
        char buf[8];
        switch (pid) {
        case PID_RPM:
            lcd.setCursor(64, 0);
            lcd.setFontSize(FONT_SIZE_XLARGE);
            lcd.printInt((unsigned int)value % 10000, 4);
            break;

        case PID_SPEED:

            if (lastSpeed != value) {
                lcd.setCursor(0, 0);
                lcd.setFontSize(FONT_SIZE_XLARGE);
                lcd.printInt((unsigned int)value % 1000, 3);
                lastSpeed = value;
            }
            break; //End of PID SPEED case.

        case PID_THROTTLE: //Will work depending on Car Specs.
            lcd.setCursor(24, 5);
            lcd.setFontSize(FONT_SIZE_SMALL);
            lcd.printInt(value % 100, 3);
            break;


        case PID_INTAKE_TEMP: //Will work depending on Car Specs.
            if (value < 1000) {
                lcd.setCursor(102, 5);
                lcd.setFontSize(FONT_SIZE_SMALL);
                lcd.printInt(value, 3);
            }
            break;
        }
    }


// Deleted #if USE_MPU6050.
//MPU6050 sensor: Measues Gyro.
//Method within if statement was showGForce.
//This portion is useless.


    void initLoggerScreen()
    {
        lcd.clear();
        lcd.backlight(true);
        lcd.setFontSize(FONT_SIZE_SMALL);
        lcd.setCursor(24, 3);
        lcd.print("kph"); //Speed: Kilometer Per Hour.
        lcd.setCursor(110, 3);
        lcd.print("rpm"); //Engine Rotates Per Min.
        lcd.setCursor(0, 5);
        lcd.print("THR:   %"); //Throttle Postion.
        lcd.setCursor(80, 5);
        lcd.print("AIR:   C"); //Air Tempurature in C.


    }
}; //End of class COBDLogger


static COBDLogger logger;


void setup()
{
    lcd.begin();
    lcd.setFontSize(FONT_SIZE_MEDIUM);
    lcd.print("Nanologger"); //Welcome Screen.


    logger.begin();
    logger.initSender();


#if ENABLE_DATA_LOG
    lcd.setFontSize(FONT_SIZE_MEDIUM);
    lcd.setCursor(0, 2);
    logger.checkSD();
#endif
    logger.setup();
}


void loop()
{
    logger.loop();
}