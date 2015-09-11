/*
  $1 Solar Soil Sensor Receiver Example with RTC
  Using DS3232 (or DS1307) RTC
  Receives data from the transmitter and prints it to serial in CSV format
  
  Connect RTC to I2C pins
  RTC and RF modules can be powered via digital pins
*/

#include <VirtualWire.h>

#include <DS3232RTC.h>
#include <Time.h>
#include <Wire.h>

long v;
int t;
int m;
int b;

void setup()
{
  Serial.begin(9600);

  // Initialise the IO and ISR
  vw_set_rx_pin(11);
  vw_setup(2000);	 // Bits per sec
  vw_rx_start();       // Start the receiver PLL running

  pinMode(A2, OUTPUT); //power RTC
  pinMode(A3, OUTPUT);
  digitalWrite(A3, HIGH);
  
  pinMode(12, OUTPUT); //power RF
  pinMode(9, OUTPUT);
  digitalWrite(12, HIGH);
    
  pinMode(13, OUTPUT); //LED

  setSyncProvider(RTC.get);
  if(timeStatus() != timeSet) 
        Serial.println("Unable to sync with the RTC");
  Serial.println("Time,Vcc,Temp,Moisture,Vbatt");
}

void loop()
{
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;

  if (vw_get_message(buf, &buflen)) // Non-blocking
  {
    int i;
    digitalWrite(13, true); // Flash a light to show received good message
    
    memcpy(&v, buf, sizeof(long));
    memcpy(&t, buf+sizeof(long), sizeof(int));
    memcpy(&m, buf+sizeof(long)+sizeof(int), sizeof(int));
    memcpy(&b, buf+sizeof(long)+2* sizeof(int), sizeof(int));

    float volts = v/1000.0;
    float vbatt = volts*b/1024;

    Serial.print(month());
    Serial.print('/');
    Serial.print(day());
    Serial.print('/');
    Serial.print(year());
    Serial.print(' ');
    Serial.print(hour());
    Serial.print(':');
    Serial.print(minute());
    Serial.print(':');
    Serial.print(second());
    Serial.print(',');
    Serial.print(volts);
    Serial.print(',');
    Serial.print(t);
    Serial.print(',');
    Serial.print(m);
    Serial.print(',');
    Serial.println(vbatt);

    digitalWrite(13, false);
  }
}
