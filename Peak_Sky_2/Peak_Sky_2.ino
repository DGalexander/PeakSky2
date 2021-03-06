// Code based on and worked by JDTanner https://github.com/jdtanner/peaksky/blob/master/peaksky_gps/peaksky_gps.ino
//Include header files required for sketch
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <util/crc16.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>

//Make definitions
//#define RADIO_SPACE_PIN 10 //Not needed if doing PWM
//#define RADIO_MARK_PIN 11 //Not needed if doing PWM
#define ENABLE_RTTY 12
#define ASCII 7
#define BAUD 50
#define INTER_BIT_DELAY (1000/BAUD)
#define PWM_PIN 9
#define ONE_WIRE_BUS 0 
#define TEMPERATURE_PRECISION 12 //12-bit precision for OneWire sensors i.e. 2dp



//Define some variables to hold GPS data
unsigned long date, time, age;
int hour, minute, second, numberOfSatellites, iteration = 1, transmitCheck;
long int gpsAltitude;
char latitudeBuffer[8], longitudeBuffer[8], timeBuffer[] = "00:00:00", transmitBuffer[128], insideTempBuffer[7], outsideTempBuffer[7];
float floatLatitude, floatLongitude, outsideTemp, insideTemp, bmpTemp, resistorDivider, batteryVoltage, r1=10000.0, r2=1500.0;


char incomingByteArray[300];
//Create a new TinyGPS object
TinyGPS gps;

//Define the pins on which the software serial will work Rx=2, Tx=3
//Create a new OneWire object on specific pin
OneWire oneWire(ONE_WIRE_BUS);

//Pass the reference to the OneWire object to Dallas Temperature
 DallasTemperature sensors(&oneWire);

//Setup arrays to hold OneWire device addresses
DeviceAddress insideThermometer, outsideThermometer;

//Create new BMP085 sensor object
Adafruit_BMP085 bmp;

         

         
//Setup function of Arduino
//Setup function of Arduino

void setup() {
 
  //Set up RTTY pins
  //pinMode(RADIO_MARK_PIN,OUTPUT); //Not needed if doing PWM
  //pinMode(RADIO_SPACE_PIN,OUTPUT); //Not needed if doing PWM
  
  //Mods for David's arduino
  int rxPin = 19;                    // RX PIN
  int txPin = 18;                    // TX TX 
 
  //Set up pin to enable radio, PWM pin, and PWM frequency
  pinMode(ENABLE_RTTY,OUTPUT);
  pinMode(PWM_PIN, OUTPUT);
 //setPwmFrequency(PWM_PIN, 1);
  int myEraser = 7;             // this is 111 in binary and is used as an eraser
  TCCR2B &= ~myEraser;         // this operation (AND plus NOT),  set the three bits in TCCR2B to 0
  int myPrescaler = 1;         // this could be a number in [1 , 6].
  TCCR2B |= myPrescaler;      //this operation (OR), replaces the last three bits in TCCR2B with our new value 011
  
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  
    //Start up OneWire sensors, discover their addresses, and set their precision
  sensors.begin();
  sensors.getAddress(insideThermometer, 0);
  sensors.getAddress(outsideThermometer, 1);
  sensors.setResolution(insideThermometer, TEMPERATURE_PRECISION);
  sensors.setResolution(outsideThermometer, TEMPERATURE_PRECISION);
 
  //Start up hardware serials at 9600 baud
  Serial1.begin(9600);
  Serial.begin(9600);
  delay(2000);

  //Set the GPS into airborne mode
  uint8_t setNav[] = {
    0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xDC};
  sendUBX(setNav, sizeof(setNav)/sizeof(uint8_t));
  getUBX_ACK(setNav);
 Serial.println("Disable GPS data");
  //Disable GPS data that we don't need
  Serial1.print("$PUBX,40,GLL,0,0,0,0*5C\r\n");
  delay(500);
  Serial1.print("$PUBX,40,ZDA,0,0,0,0*44\r\n");
  delay(500);
  Serial1.print("$PUBX,40,VTG,0,0,0,0*5E\r\n");
  delay(500);
  Serial1.print("$PUBX,40,GSV,0,0,0,0*59\r\n");
  delay(500);
  Serial1.print("$PUBX,40,GSA,0,0,0,0*4E\r\n");
  delay(500);
  Serial1.print("$PUBX,40,RMC,0,0,0,0*47\r\n");
  delay(500);
  Serial1.print("$PUBX,40,GGA,0,0,0,0*5A\r\n");
  delay(500);

  //Give the GPS time to breathe :)
  delay(2500);
}

//Loop function of Arduino
void loop() {

  //Request NMEA sentence from GPS
  Serial1.print("$PUBX,00*33\r\n");
Serial.println("Request NMEA sentence from GPS");
  //GPS does not respond immediately, so give it 1.5 seconds
  delay(1500);

  while (Serial1.available() > 0) {
     int j=0;
    //int c = Serial1.read();
    incomingByteArray[j] = Serial1.read();

    //Pass TinyGPS integer values of each character recieved from the GPS and encode
    //int checkNMEASentence = gps.encode(c);
    int checkNMEASentence = gps.encode(incomingByteArray[j]);
    j++;
    Serial.print(incomingByteArray);
    //Only if TinyGPS has received a complete NMEA sentence
    if (checkNMEASentence > 0) {

      //Query the TinyGPS object for the number of satellites
      numberOfSatellites = gps.sats();
      Serial.println(numberOfSatellites);
      //Query the TinyGPS object for the date, time and age
      gps.get_datetime(&date, &time, &age);

      //Convert the time to something useful
      hour = (time / 1000000);
      minute = ((time - (hour * 1000000)) / 10000);
      second = ((time - ((hour * 1000000) + (minute * 10000))));
      second = second / 100;

      if (numberOfSatellites >= 1) {
        digitalWrite(ENABLE_RTTY, HIGH);

        //Get Position
        gps.f_get_position(&floatLatitude, &floatLongitude);
        
         //Get temperatures from all OneWire sensors 
        sensors.requestTemperatures();
        outsideTemp = sensors.getTempC(outsideThermometer);
        insideTemp = sensors.getTempC(insideThermometer);
             
        //Convert float to string
        dtostrf(floatLatitude, 7, 4, latitudeBuffer);
        dtostrf(floatLongitude, 7, 4, longitudeBuffer);
        dtostrf(outsideTemp, 6, 2, outsideTempBuffer);
        dtostrf(insideTemp, 6, 2, insideTempBuffer);
        
        //Check that we are putting a space at the front of lonbuf
        if(longitudeBuffer[0] == ' ')
        {
          longitudeBuffer[0] = '+';
        }

        //Convert altitude to metres
        gpsAltitude = (gps.altitude() / 100);

        //Construct the transmit buffer
        sprintf(transmitBuffer, "$$PEAKSKY,%d,%02d:%02d:%02d,%s,%s,%ld,%d,%s,%s,", iteration, hour, minute, second, latitudeBuffer, longitudeBuffer, gpsAltitude, numberOfSatellites, outsideTempBuffer, insideTempBuffer);

        if (transmitCheck > -1) {

          //Append the CRC16 checksum to the end of the transmit buffer
          transmitCheck = sprintf (transmitBuffer, "%s*%04X\n", transmitBuffer, gps_CRC16_checksum(transmitBuffer));

          //Pass the transmit buffer to the RTTY function
          Serial.print(transmitBuffer);
          rtty_txstring(transmitBuffer);                                
        }
        iteration++;        
      }
    }
  }
}

//----------------------------------------------------                                                      
//HELPER FUNCTIONS
//----------------------------------------------------                                                      
void sendUBX(uint8_t *MSG, uint8_t len) {           // Send a byte array of UBX protocol to the GPS
  for(int i=0; i<len; i++) {
    Serial1.write(MSG[i]);
  }
}
//----------------------------------------------------                                                      
boolean getUBX_ACK(uint8_t *MSG) {                  // Calculate expected UBX ACK packet and parse UBX response from GPS
  uint8_t b;
  uint8_t ackByteID = 0;
  uint8_t ackPacket[10];
  unsigned long startTime = millis();
  // Construct the expected ACK packet    
  ackPacket[0] = 0xB5;                             // header
  ackPacket[1] = 0x62;	                            // header
  ackPacket[2] = 0x05;	                            // class
  ackPacket[3] = 0x01;	                            // id
  ackPacket[4] = 0x02;	                            // length
  ackPacket[5] = 0x00;
  ackPacket[6] = MSG[2];	                    // ACK class
  ackPacket[7] = MSG[3];	                    // ACK id
  ackPacket[8] = 0;		                    // CK_A
  ackPacket[9] = 0;		                    // CK_B

  for (uint8_t i=2; i<8; i++) {                     // Calculate the checksums
    ackPacket[8] = ackPacket[8] + ackPacket[i];
    ackPacket[9] = ackPacket[9] + ackPacket[8];
  }
  while (1) {
    if (ackByteID > 9) {                            // Test for success
      return true;
    }
    if (millis() - startTime > 3000) {              // Timeout if no valid response in 3 seconds 
      return false;
    }
    if (Serial1.available()) {                           // Make sure data is available to read
      b = Serial1.read();
      if (b == ackPacket[ackByteID]) {              // Check that bytes arrive in sequence as per expected ACK packet 
        ackByteID++;
      } 
      else {
        ackByteID = 0;                              // Reset and look again, invalid order
      }
    }
  }
}
//----------------------------------------------------                                                      
uint16_t gps_CRC16_checksum(char *string) {         // CRC16 checksum function
  size_t i;
  uint16_t crc;
  uint8_t c;
  crc = 0xFFFF;                                                  
  for (i = 2; i < strlen(string); i++)              // Calculate checksum ignoring the first two $s
  {
    c = string[i];
    crc = _crc_xmodem_update (crc, c);
  }
  return crc;
}
//----------------------------------------------------                                                      
void rtty_txstring(char *string) {                  // Transmit a string, one char at a time		
  int i;					    // Define disposable integer counter variable
  for (i = 0; i < strlen(string); i++)	            // Iterate over the string array
  {
    //rtty_txbyte(string[i]);			    // Pass each element of the string array to rtty_txbyte; not required if using PWM
    rtty_pwmtxbyte(string[i]);			    // Pass each element of the string array to rtty_pwmtxbyte
  }
}
//----------------------------------------------------                                                      
void setPwmFrequency(int pin, int divisor) {        // See http://arduino.cc/playground/Code/PwmFrequency
  byte mode;
  if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch(divisor) {
    case 1:
      mode = 0x01;
      break;
    case 8:
      mode = 0x02;
      break;
    case 64:
      mode = 0x03;
      break;
    case 256:
      mode = 0x04;
      break;
    case 1024:
      mode = 0x05;
      break;
    default:
      return;
    }
    if(pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    }
    else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  }
  else if(pin == 3 || pin == 11) {
    switch(divisor) {
    case 1:
      mode = 0x01;
      break;
    case 8:
      mode = 0x02;
      break;
    case 32:
      mode = 0x03;
      break;
    case 64:
      mode = 0x04;
      break;
    case 128:
      mode = 0x05;
      break;
    case 256:
      mode = 0x06;
      break;
    case 1024:
      mode = 0x7;
      break;
    default:
      return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}
//----------------------------------------------------                                                      
void rtty_pwmtxbyte(char c) {                       // Convert each character byte to bits
  int i;                                            // Define disposible integer counter variable
  rtty_pwmtxbit(0); 				    // Start bit
  for (i=0;i<ASCII;i++)				    // 7-bit ascii (see DEFINE above)
  {
    if (c & 1) rtty_pwmtxbit(1); 		    // Starting with least significant bit, if bit=1 then transmit MARK 
    else rtty_pwmtxbit(0);			    // If bit=0 then transmit SPACE
    c = c >> 1;                                     // Shift along to the next bit
  }
  rtty_pwmtxbit(1);                                 // Stop bit
  rtty_pwmtxbit(1);                                 // Stop bit                 
}
//----------------------------------------------------                                                      
void rtty_pwmtxbit(int bit) {                       // Transmit individual bits using PWM
  analogWrite(PWM_PIN,(bit>0)?102:90);
  delay(INTER_BIT_DELAY);                           // 50 bits per second i.e. baudrate
}
//----------------------------------------------------                                                      
//This section isn't required if using PWM                    
//----------------------------------------------------                                                      
//void rtty_txbyte(char c) {                        // Convert each character byte to bits
//  int i;                                          // Define disposible integer counter variable
//  rtty_txbit(0); 				    // Start bit
//  for (i=0;i<ASCII;i++)			    // 7-bit ascii (see DEFINE above)
//  {
//    if (c & 1) rtty_txbit(1); 		    // Starting with least significant bit, if bit=1 then transmit MARK 
//    else rtty_txbit(0);			    // If bit=0 then transmit SPACE
//    c = c >> 1;                                   // Shift along to the next bit
//  }
//  rtty_txbit(1);                                  // Stop bit
//  rtty_txbit(1);                                  // Stop bit                 
//}
//----------------------------------------------------                                                      
//void rtty_txbit(int bit) {                        // Transmit individual bits
//  digitalWrite(RADIO_MARK_PIN,(bit>0)?HIGH:LOW);
//  digitalWrite(RADIO_SPACE_PIN,(bit>0)?LOW:HIGH);
//  delay(INTER_BIT_DELAY);                         // 50 bits per second i.e. baudrate
//}
//----------------------------------------------------
