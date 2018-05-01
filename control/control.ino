
#include "LiquidCrystal.h"
#include <EEPROM.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7); 

// which pin to use for reading the sensor? can use any pin!
#define FLOWSENSORPIN 3
#define VALVEPIN 12
#define LOWERSENSORPIN 3
#define HIGHERSENSORPIN 5

bool reset = false;
int lower, upper;
unsigned long currentTime;
unsigned long cloopTime;
bool isValveOpen = false;
float lastState = 0;


// count how many pulses!
volatile long pulses = 0;
// track the state of the pulse pin
volatile uint8_t lastflowpinstate;
// you can try to keep time of how long it is between pulses
volatile uint32_t lastflowratetimer = 0;
// and use that to calculate a flow rate
volatile float flowrate;
// Interrupt is called once a millisecond, looks for any pulses from the sensor!
SIGNAL(TIMER0_COMPA_vect) {
  uint8_t x = digitalRead(FLOWSENSORPIN);
  
  if (x == lastflowpinstate) {
    lastflowratetimer++;
    return; // nothing changed!
  }
  
  if (x == HIGH) {
    //low to high transition!
    pulses++;
  }
  lastflowpinstate = x;
  lastflowratetimer = 0;
}

void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
  } else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
  }
}

void EEPROMWritelong(int address, long value)
      {
      //Decomposition from a long to 4 bytes by using bitshift.
      //One = Most significant -> Four = Least significant byte
      byte four = (value & 0xFF);
      byte three = ((value >> 8) & 0xFF);
      byte two = ((value >> 16) & 0xFF);
      byte one = ((value >> 24) & 0xFF);

      //Write the 4 bytes into the eeprom memory.
      EEPROM.write(address, four);
      EEPROM.write(address + 1, three);
      EEPROM.write(address + 2, two);
      EEPROM.write(address + 3, one);
      }


long EEPROMReadlong(long address)
      {
      //Read the 4 bytes from the eeprom memory.
      long four = EEPROM.read(address);
      long three = EEPROM.read(address + 1);
      long two = EEPROM.read(address + 2);
      long one = EEPROM.read(address + 3);

      //Return the recomposed long by using bitshift.
      return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
      }

void clearLine(int line){
  lcd.setCursor(0, line);
  lcd.print("                ");
}

void setup() {
   Serial.begin(9600);
   Serial.print("Flow sensor test!");
   lcd.begin(16, 2);
   
   pinMode(VALVEPIN, OUTPUT);
   
   pinMode(FLOWSENSORPIN, INPUT);
   digitalWrite(FLOWSENSORPIN, HIGH);

   pulses = EEPROMReadlong(0);
   lastState = pulses;
   lastflowpinstate = digitalRead(FLOWSENSORPIN);
   useInterrupt(true);
}

void volumeControl()
{
 
  
  //Serial.print("Freq: "); Serial.println(flowrate);
  //Serial.print("Pulses: "); Serial.println(pulses, DEC);
  
  // if a plastic sensor use the following calculation
  // Sensor Frequency (Hz) = 7.5 * Q (Liters/min)
  // Liters = Q * time elapsed (seconds) / 60 (seconds/minute)
  // Liters = (Frequency (Pulses/second) / 7.5) * time elapsed (seconds) / 60
  // Liters = Pulses / (7.5 * 60)
  float liters = pulses;
  liters /= 7.5;
  liters /= 60.0;

   // запись текущего состояния
   if(1 <= abs(liters - lastState))
   {
      EEPROMWritelong(0, long(pulses));
      lastState = liters;
   }

  
  Serial.print(liters); Serial.println(" Liters");
  if(!reset)
  {
    clearLine(0);
    lcd.setCursor(0, 0);
    lcd.print("Volume:");
    clearLine(1);
    lcd.setCursor(0, 1);
    lcd.print(liters); lcd.print(" Liters        ");
  }
}

void valveControl()
{
      lower = analogRead(LOWERSENSORPIN);
      upper = analogRead(HIGHERSENSORPIN);
      //Serial.println(String(lower) + " " + String(upper));
      if(lower > 1000 && upper > 1000)
      {
          digitalWrite(VALVEPIN, true); //open
      }
      else if(lower < 100 && upper < 100)
      {
          digitalWrite(VALVEPIN, false); //close
      }
      
      
}
int detectButton() {
  int keyAnalog =  analogRead(A0);
  //Serial.println(keyAnalog);
  if(keyAnalog > 630 && keyAnalog < 650 && !reset)
  {
    clearLine(0);
    lcd.setCursor(0, 0);
    lcd.print("Reset volume?");
    clearLine(1);
    lcd.setCursor(0, 1);
    lcd.print("Y/N");
    reset = true;
  }
  if(keyAnalog < 10 && reset)
  {
    reset = false;
  }
  if(keyAnalog > 400 && keyAnalog < 420 && reset) // Confirm clear
  {
    pulses = 0; 
    reset = false;
  }
  
}



void loop()                     // run over and over again
{ 
    valveControl();
    currentTime = millis();
    
    if(currentTime >= (cloopTime + 1000))
    {     
      cloopTime = currentTime;              // Updates cloopTime
      volumeControl();
    }

    
    
    detectButton();
  
}
