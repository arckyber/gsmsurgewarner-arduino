/*
    GPRS SMS Read

    This sketch is used to test seeeduino GPRS_Shield's reading SMS
    function.To make it work, you should insert SIM card
    to Seeeduino GPRS Shield,enjoy it!

    There are two methods to read SMS:
    1. GPRS_LoopHandle.ino  -> in order to recieve "+CMTI: "SM""
      may be you need to send this command to your shield: "AT+CNMI=2,2,0,0,0"
    2. GPRS_SMSread.ino -> you have to check if there are any
      UNREAD sms, and you don't need to check serial data continuosly

    create on 2015/05/14, version: 1.0
    by op2op2op2(op2op2op2@hotmail.com)
*/

/*
 * SENDING, RECEIVING SMS PROTOCOL  => THIS MUST ALSO BE DOCUMENTED IN THE SERVER
 * 
 * MESSAGE FORMAT IN SENDING SMS (from python to arduino)
 * RECEIVER&CMD&SIM_NUMBER&MESSAGE&DATETIME
 * 
 * Ex. RECEIVER&SEND&+6309506305568&Good day!&12/12/2021,15:23
 * 
 * MESSAGE FORMAT IN RECEIVING SMS  (from arduino to python)
 * TRANSMITTER:CMD:SIM_NUMBER:MESSAGE:DATETIME
 * 
 * Ex. TRANSMITTER&SAVE&+6309506305568&Good day!&12/12/2021,15:23
 */

#include "GPRS_Shield_Arduino.h"
#include <SoftwareSerial.h>
#include <Wire.h>
//#include <String.h>

#define PIN_TX    7
#define PIN_RX    8
#define BAUDRATE  9600

const char SEPARATOR = '&';

const String SEND = "SEND";
const String SAVE = "SAVE";
const String TRANSMITTER = "TRANSMITTER";
const String RECEIVER = "RECEIVER";

const String OTHERS = "OTHERS";

#define MESSAGE_LENGTH 160
char message[MESSAGE_LENGTH];
int messageIndex = 0;

char phone[16];
char datetime[24];

#define SOURCE_INDEX 0
#define CMD_INDEX 1
#define SIMNUMBER_INDEX 2
#define MSG_INDEX 3
#define DATETIME_INDEX 4

GPRS gprs(PIN_TX, PIN_RX, BAUDRATE); //RX,TX,PWR,BaudRate

void setup() {
    gprs.checkPowerUp();
    Serial.begin(9600);
    while (!gprs.init()) {
//        Serial.print("init error\r\n");
        delay(1000);
    }

    while (!gprs.isNetworkRegistered()) {
        delay(1000);
//        Serial.println("Network has not registered yet!");
    }
    
    delay(3000);
//    Serial.println("Init Success, please send SMS message to me!");
}

void loop() {
    delay(500);
    if (Serial.available() > 0)
    {
      String data = Serial.readString();
      String source = getValue(data, SEPARATOR, SOURCE_INDEX);
      String cmd = getValue(data, SEPARATOR, CMD_INDEX);
      String phone_num = getValue(data, SEPARATOR, SIMNUMBER_INDEX);
      String msg = getValue(data, SEPARATOR, MSG_INDEX);
      if (cmd.equals(SEND) && source.equals(RECEIVER)) //Send message
      {
        // Send message here....................TO TRANSMITTER
        if (gprs.sendSMS(phone_num.c_str(), msg.c_str())) { //define phone number and text
//            Serial.print("Send SMS Succeed!\r\n");
        } else {
//            Serial.print("Send SMS failed!\r\n");
        }
      }
      else
      {
        // Send message here................. to phone number
      }
    }
    messageIndex = gprs.isSMSunread();
    if (messageIndex > 0) { //At least, there is one UNREAD SMS
        gprs.readSMS(messageIndex, message, MESSAGE_LENGTH, phone, datetime);
        //In order not to full SIM Memory, is better to delete it
        gprs.deleteSMS(messageIndex);        
//      Read message here and pass to python .............

        String str = String(message);

        int i = str.indexOf(TRANSMITTER);

        if (str.indexOf(TRANSMITTER) != -1)
        {
          String data = TRANSMITTER;
          data.concat(SEPARATOR);
          data.concat(SAVE);
          data.concat(SEPARATOR);
          data.concat(phone);
          data.concat(SEPARATOR);
          data.concat(message);
          data.concat(SEPARATOR);
          data.concat(datetime);
          delay(50);
          Serial.print(data);
        }
        else 
        {
          String data = OTHERS;
          data.concat(SEPARATOR);
          data.concat(SAVE);
          data.concat(SEPARATOR);
          data.concat(phone);
          data.concat(SEPARATOR);
          data.concat(message);
          data.concat(SEPARATOR);
          data.concat(datetime);
          delay(50);
          Serial.print(data);
        }

    }
}



String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}
