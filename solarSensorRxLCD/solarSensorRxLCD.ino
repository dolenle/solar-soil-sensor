/*
  $1 Solar Soil Sensor Receiver Example
  Receives data from the transmitter and prints it to serial,
  and displays it on a 20x4 HD44780 LCD
  
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
 
 * 433MHZ receiver module data to digital pin 6
*/

#include <LiquidCrystal.h>
#include <VirtualWire.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

long v;
int t;
int m;
int b;

void setup()
{
  Serial.begin(9600);
  Serial.println("Solar RX");

  // Initialise the IO and ISR
  vw_set_rx_pin(6);
  vw_setup(2000);  // Bits per sec
  vw_rx_start();       // Start the receiver PLL running
  
  lcd.begin(20, 4);
  lcd.print("Hello, World!");
}

void loop()
{
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;

  if (vw_get_message(buf, &buflen)) // Non-blocking
  {
    lcd.clear();
    digitalWrite(13, true); // Flash a light to show received good message
    
    memcpy(&v, buf, sizeof(long));
    memcpy(&t, buf+sizeof(long), sizeof(int));
    memcpy(&m, buf+sizeof(long)+sizeof(int), sizeof(int));
    memcpy(&b, buf+sizeof(long)+2* sizeof(int), sizeof(int));

    float vbat = b/1000.0*v/1024;
    
    lcd.print("Vcc = ");
    lcd.print(v/1000.0);
    lcd.setCursor(0,1);
    lcd.print("Temp = ");
    lcd.print(t);
    lcd.setCursor(0,2);
    lcd.print("Moisture = ");
    lcd.print(m);
    lcd.setCursor(0,3);
    lcd.print("Vbat = ");
    lcd.print(vbat);

    Serial.print("Vcc = ");
    Serial.print(v/1000.0);
    Serial.print("\tTemp = ");
    Serial.print(t);
    Serial.print("\tMoisture = ");
    Serial.print(m);
    Serial.print("\tVbat = ");
    Serial.println(vbat);
    
    digitalWrite(13, false);
  }
}
