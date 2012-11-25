// DA Combined Sensor Serial Print PEAK SKY II
// September 2012
// Barometric Pressure Sensor BMP085
// Internal Temp Sensor TMP 36
// GPS UBLOX 6

// Include header files
#include "Wire.h"
#include "Adafruit_BMP085.h"

Adafruit_BMP085 bmp;

//char incomingByte;
char incomingByteArray[300];
//Define the pins on which the software serial will work Rx=19, Tx=18
int rxPin = 19;                    // RX PIN
int txPin = 18;                    // TX TX

// Tie 3.3V to ARef 
#define aref_voltage 3.3 

//TMP36 Pin Variables
int tempPin = 0;        //the analog pin the TMP36's Vout (sense) pin is connected to
                        //the resolution is 10 mV / degree centigrade with a
                        //500 mV offset to allow for negative temperatures
int tempReading;        // the analog reading from the sensor

void setup(void) {
  // Send debugging information via the Serial monitor
  Serial.begin(9600);   
 
  // Set the aref to something other than 5v
  analogReference(EXTERNAL);
  
  // Start Adafruit
   bmp.begin();
   
  //Start up software and hardware serial1 at 9600 baud
  //gps.begin(9600);
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);  
  Serial1.begin(9600);
  delay(5000);
  //Disable data that we don't need.
  Serial.println("$PUBX disable GLL....");
  Serial1.print("$PUBX,40,GLL,0,0,0,0*5C\r\n");
  Serial.println("$PUBX disable ZDA....");
  Serial1.print("$PUBX,40,ZDA,0,0,0,0*44\r\n");
  Serial.println("$PUBX disable VTG....");
  Serial1.print("$PUBX,40,VTG,0,0,0,0*5E\r\n");
  Serial.println("$PUBX disable GSV....");
  Serial1.print("$PUBX,40,GSV,0,0,0,0*59\r\n");
  Serial.println("$PUBX disable GSA....");
  Serial1.print("$PUBX,40,GSA,0,0,0,0*4E\r\n");
  Serial.println("$PUBX disable RMC....");
  Serial1.print("$PUBX,40,RMC,0,0,0,0*47\r\n");
  Serial.println("$PUBX disable GGA....");
  Serial1.print("$PUBX,40,GGA,0,0,0,0*5A\r\n");
  //Give the GPS time to breathe :)
  delay(5000);   
 
}
   
void loop(void) {
  // loop functions for bmp sensor
  Serial.print("Temperature = ");
  Serial.print(bmp.readTemperature());
  Serial.println(" *C");
    
  Serial.print("Pressure = ");
  Serial.print(bmp.readPressure());
  Serial.println(" Pa");
    
  // Calculate altitude assuming 'standard' barometric
  // pressure of 1013.25 millibar = 101325 Pascal
  Serial.print("Altitude = ");
  Serial.print(bmp.readAltitude());
  Serial.println(" meters");

  Serial.print("Real altitude = ");
  Serial.print(bmp.readAltitude(102800));  // Configure actual pressure here -- Pascals
  Serial.println(" meters");
    
  Serial.println();
  delay(1000);
    
  // loop functions for external temps sensor
  tempReading = analogRead(tempPin);  
 
  Serial.print("Temp reading = ");
  Serial.print(tempReading);     // the raw analog reading
 
  // converting that reading to voltage, which is based off the reference voltage
  float voltage = tempReading * aref_voltage;
  voltage /= 1024.0; 
 
  // print out the voltage
  Serial.print(" - ");
  Serial.print(voltage); Serial.println(" volts");
 
  // now print out the temperature
  float temperatureC = (voltage - 0.5) * 100 ;  //converting from 10 mv per degree wit 500 mV offset
                                               //to degrees ((volatge - 500mV) times 100)
  Serial.print(temperatureC); Serial.println(" degrees C");
 
  // now convert to Fahrenheight
  float temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;
  Serial.print(temperatureF); Serial.println(" degrees F");
 
  delay(1000);
  
  Serial.println("Entering loop");
  Serial1.print("$PUBX,00*33\r\n");
  Serial.println("Executed GPS request");
  int i;
  Serial.print(i);
  while (Serial1.available() > 0) {
  incomingByteArray[i] = Serial1.read();
  i++;
  }
  {
  Serial.println(incomingByteArray); 
  delay(1000);

  }

}
  
