/*
 * ATTiny85 Solar Powered Wireless Moisture Sensor
 * Dolen Le
 * 
 * Code for the transmitter end. For ATTiny85 only.
 */

#include <VirtualWire.h>
#include <avr/sleep.h>
//#include <SoftwareSerial.h>

#define SLEEP_CYCLES 2 //sleep cycles between transmissions
#define TEMP_OFFSET 22 //temp sensor calibration

#define LED_PIN 0 //to indicator LED
#define TX_PIN 1 //to RF
#define BOOST_PIN 4 //to MOSFET boost converter control
#define SENSOR_PIN A1 //to moisture sensor
#define VBAT_PIN A3 //to battery positive

struct data {
  long vcc;
  int temp;
  int moisture;
  int vbat;
};

uint8_t txBuf[sizeof(struct data)];
struct data measure;
int wdiCount = 0; //sleep counter

void setup() {
  //setup virtualwire
  vw_set_tx_pin(TX_PIN);
  vw_setup(2000);
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(BOOST_PIN, OUTPUT);
  pinMode(SENSOR_PIN, INPUT); //Moisture sensor pin

  //Power Saving Options
  //ATTiny safety level 1 allows watchdog settings to be changed at any time
  WDTCR = (1<<WDIE) | (0<<WDE) | (1<<WDP3) | (0<<WDP2) | (0<<WDP1) | (1<<WDP1); //1001 = 8 sec

  //Disable all digital input buffers; no digital inputs needed
  DIDR0 = 0x3F;

  //Disable the anlog comparator (not used)
  ACSR |= (1<<ACD);

  //Disable Timer1 and the USI (not used)
  PRR = 0x0A;
}

void loop() {
  measure.vcc = readVcc();
  if(wdiCount >= SLEEP_CYCLES) {
    digitalWrite(SENSOR_PIN, HIGH); //enable sensor pullup
    digitalWrite(BOOST_PIN, HIGH); //enable boost converter
    
    delayMicroseconds(500); //Wait for stablization
    measure.moisture = analogRead(SENSOR_PIN);
    digitalWrite(SENSOR_PIN, LOW); //disable sensor pullup
    
    measure.vbat = analogRead(VBAT_PIN);
    
    measure.temp = readTemp();
    
    digitalWrite(LED_PIN, HIGH); //flash LED
    
    //Copy measurements to transmit buffer
    memcpy(&txBuf, &measure, sizeof(measure));
    vw_send(txBuf, sizeof(measure)); //begin transmit
    vw_wait_tx(); //Wait for transmission
        
    digitalWrite(LED_PIN, LOW);
    wdiCount = 0; //reset sleep counter
  } else {
    if(measure.vcc<3000) {
      digitalWrite(BOOST_PIN, HIGH); //recharge capacitor
    } else if(measure.vcc>5000) {
      digitalWrite(BOOST_PIN, LOW); //stop recharging
    }
  }
  doSleep();
}

long readVcc() {
  //Use Vcc as reference and connect 1.1V source to ADC
  ADMUX = (1<<MUX3) | (1<<MUX2); //ATTINY85 ONLY
 
  delayMicroseconds(3000); //settling time
  ADCSRA |= (1<<ADSC);  //start conversion
  while (bit_is_set(ADCSRA,ADSC)); //wait for ADC

  return 1030200L / ADC; //Vcc in mV; default 1125300 = 1.1*1023*1000 1030232L
}

int readTemp() {
  //Set MUX to 1111 and use 1.1V as reference
  ADMUX = (1<<REFS1) | 0x0F;
  
  delayMicroseconds(3000); //settling time
  ADCSRA |= (1<<ADSC); //start conversion
  while (bit_is_set(ADCSRA,ADSC)); //wait for ADC
  
  return ADC-273+TEMP_OFFSET;
}


void doSleep() {
  ADCSRA &= ~(1<<ADEN); //disable ADC
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); //deepest sleep mode
  sleep_enable(); //enable sleep in MCUSR
  sleep_mode(); //sleep here
  sleep_disable(); //wake here
  ADCSRA |= (1<<ADEN); //reenable ADC
}

//Watchdog ISR
ISR(WDT_vect) {
  wdiCount++; //increment sleep counter
}
