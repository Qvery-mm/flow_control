#include <LiquidCrystal.h>
#include <EEPROM.h> 

#define OPEN true
#define CLOSE false

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

volatile int  flow_frequency;  // Measures flow meter pulses                 
unsigned char flowmeter = 3;  // Flow Meter Pin number
unsigned long currentTime;
unsigned long cloopTime;

unsigned long volume = 0;
unsigned long lastVolume = 0;
unsigned long limit = 4000000;

int lowerPin = 3;
int upperPin = 5;
int valvePin = 11;
int lower, upper;
bool reset = false;

void setup()
{ 
   volume = EEPROMReadlong(0);
   lastVolume = volume;
   
   lcd.begin(16, 2);  
   delay(3000);
   lcd.setCursor(0, 0);
   lcd.print("Volume");

   //pinMode(flowmeter, INPUT);
   Serial.begin(9600); 
   
   pinMode(flowmeter, INPUT_PULLUP);
   
   attachInterrupt(digitalPinToInterrupt(flowmeter), flow, RISING); // Setup Interrupt 
                                     // see http://arduino.cc/en/Reference/attachInterrupt
   sei();   
   
   
   // Enable interrupts  
   currentTime = millis();
   cloopTime = currentTime;

   pinMode(valvePin, OUTPUT);
} 

void valveControl(bool isOpen)
{
   digitalWrite(valvePin, isOpen);
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
    clearLine(0);
    lcd.setCursor(0, 0);
    lcd.print("Volume");
    reset = false;
  }
  if(keyAnalog > 400 && keyAnalog < 420 && reset) // Confirm clear
  {
    clearLine(0);
    lcd.setCursor(0, 0);
    lcd.print("Volume");
    clearLine(1);
    lcd.setCursor(0, 1);
    volume = 0; 
    reset = false;
  }
  
}

void flow ()                  // Interruot function
{ 
  flow_frequency++;
  //Serial.println("interrrupt");
} 

void clearLine(int line){
  lcd.setCursor(0, line);
  lcd.print("                ");
}
 
void printDisplay(String message, int line){
  //Serial.println(message);
  clearLine(line);
  lcd.setCursor(0, line);
  lcd.print(message);
}


void volumeControl()
{
      lower = analogRead(lowerPin);
    upper = analogRead(upperPin);
   Serial.println(String(lower) +  " " + String(upper)); 
    if(lower > 1000 && upper > 1000)
       valveControl(OPEN);
    if(lower < 100 && upper < 100)
       valveControl(CLOSE);
    
}
/*
flowControl_1()
{
  //контроль потока
   currentTime = millis();
   if(currentTime >= (cloopTime + 1000))
   {     
      cloopTime = currentTime;              // Updates cloopTime
      // Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min. (Results in +/- 3% range)
      volume += (flow_frequency * 1000.0)  / 60 / 7.5;
      printDisplay(String(volume), 1);   
   }
}
*/
int l_hour = 0;
void flowControl_2()
{
    currentTime = millis();
   // Every second, calculate and print litres/hour
   if(currentTime >= (cloopTime + 1000))
   {     
      cloopTime = currentTime;              // Updates cloopTime
      // Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min. (Results in +/- 3% range)
      l_hour = (flow_frequency * 60 / 7.5); // (Pulse frequency x 60 min) / 7.5Q = flow rate in L/hour 
      flow_frequency = 0;                   // Reset Counter
      Serial.print(l_hour, DEC);            // Print litres/hour
      Serial.println(" L/hour");
   }
}

void flowControl_3()
{
   currentTime = millis();
   // Every second, calculate and print litres/hour
   if(currentTime >= (cloopTime + 1000))
   {  
      Serial.println(flow_frequency);
   }
}


void loop ()    
{
  //Serial.println(digitalRead(flow))
    //контроль наполнения
  volumeControl();
  //flowControl_3();

  
   //запись текущего состояния
//   if(1000 <= abs(volume - lastVolume))
//   {
//      EEPROMWritelong(0, volume);
//      lastVolume = volume;
//   }

   detectButton();
}
