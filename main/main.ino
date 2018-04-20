
#include <LiquidCrystal.h>
#include <EEPROM.h> 

#define OPEN true
#define CLOSE false

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

volatile int  flow_frequency;  // Measures flow meter pulses                 
unsigned char flowmeter = 2;  // Flow Meter Pin number
unsigned long currentTime;
unsigned long cloopTime;

unsigned long volume = 0;
unsigned long lastVolume = 0;
unsigned long limit = 4000000;

int lowerPin = 1;
int upperPin = 5;
int valvePin = 3;
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

   pinMode(flowmeter, INPUT);
   Serial.begin(9600); 
   attachInterrupt(0, flow, RISING); // Setup Interrupt 
                                     // see http://arduino.cc/en/Reference/attachInterrupt
   sei();                            // Enable interrupts  
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
  Serial.println(keyAnalog);
  if(keyAnalog == 640 && !reset)
  {
    clearLine(0);
    lcd.setCursor(0, 0);
    lcd.print("Reset volume?");
    clearLine(1);
    lcd.setCursor(0, 1);
    lcd.print("Y/N");
    reset = true;
  }
  if(keyAnalog == 0 && reset)
  {
    clearLine(0);
    lcd.setCursor(0, 0);
    lcd.print("Volume");
    reset = false;
  }
  if(keyAnalog == 410 && reset) // Confirm clear
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
} 

void clearLine(int line){
  lcd.setCursor(0, line);
  lcd.print("                ");
}
 
void printDisplay(String message, int line){
  Serial.println(message);
  clearLine(line);
  lcd.setCursor(0, line);
  lcd.print(message);
}

void loop ()    
{
    //контроль наполнения
    lower = analogRead(lowerPin);
    upper = analogRead(upperPin);
    Serial.println(String(lower) +  " " + String(upper)); 
    if(lower > 1000 && upper > 1000)
       valveControl(OPEN);
    if(lower < 100 && upper < 100)
       valveControl(CLOSE);

  //контроль потока
   currentTime = millis();
   
   if(currentTime >= (cloopTime + 1000))
   {     
      cloopTime = currentTime;              // Updates cloopTime
      // Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min. (Results in +/- 3% range)
      
      if(flow_frequency * 1000 / 60 / 7.5 > 0)
        volume += (flow_frequency * 1000 / 60 / 7.5);
      flow_frequency = 0;                   // Reset Counter
      if(!reset)
      {
        //check for limit
        if(volume > limit)
          printDisplay("limit exceeded!", 0);
        //перевод в литры и вывод на экран
        String liters = String(volume/1000), milliliters = String(volume % 1000);
        while(milliliters.length() < 3)
          milliliters = "0" + milliliters;
        printDisplay(liters + "." + milliliters, 1);   
      }
   }
   //запись текущего состояния
   if(1000 <= abs(volume - lastVolume))
   {
      EEPROMWritelong(0, volume);
      lastVolume = volume;
   }

   detectButton();
}
