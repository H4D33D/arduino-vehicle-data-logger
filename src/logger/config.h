#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED


//Data logging/streaming out
#define ENABLE_DATA_OUT 0
#define ENABLE_DATA_LOG 1 //Data logging enabled
#define USE_SOFTSERIAL 0


//format of log file
#define LOG_FORMAT FORMAT_CSV


//Choose SD pin
#define SD_CS_PIN 10 // SD breakout


//Choose LCD model
LCD_SH1106 lcd;


//Other options
//#define DEBUG Serial
#define DEBUG_BAUDRATE 9600


#endif // CONFIG_H_INCLUDED