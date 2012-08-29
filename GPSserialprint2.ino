
#include <SoftwareSerial.h>
int rxPin = 14;                    // RX PIN 
int txPin = 15;                    // TX TX

void setup()  {
 pinMode(rxPin, INPUT);
 pinMode(txPin, OUTPUT);
Serial1.begin(9600);

}

void loop () {
  
}
