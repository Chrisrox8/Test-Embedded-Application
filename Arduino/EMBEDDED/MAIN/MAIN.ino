//LIBRARIES

#include <OneWire.h>
#include <DallasTemperature.h>   
#include <LiquidCrystal.h>
#include <math.h>

//PIN ASSIGNMENT
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
#define ONE_WIRE_BUS 1
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

//SWITCHING RELAYS
const int button1Pin = 0;    // the number of relay 1's pushbutton's pin
const int button2Pin = 10;    // the number of relay 2's pushbutton's pin
const int relay1Pin = 13;    // the number of the pin for relay 1
const int relay2Pin = 7;     // the number of the pin for relay 2

//VARIABLES
int latch1 = 0;              // the latch for the first button
int latch2 = 0;              // the latch for the second button

//SETUP
void setup() {
  //START UP SENSORS
  sensors.begin();
  
// declare pin 9 to be an output:
  pinMode(9, OUTPUT);  
  analogWrite(9, 50);   
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 4);
  // Print a message to the LCD.

  //RELAY SWITCHING
  // initialize the relay pins as an output:
  pinMode(relay1Pin, OUTPUT);
  pinMode(relay2Pin, OUTPUT);
  // initialize the pushbutton pins as an input:
  pinMode(button1Pin, INPUT);
  pinMode(button2Pin, INPUT);
  digitalWrite(relay1Pin, LOW);  
  digitalWrite(relay2Pin, LOW); 
}

void loop() {
  //REQUEST TEMPERATURES
  sensors.requestTemperatures();
  
  //DISPLAY TEMP ON THE DISPLAYER
  lcd.print("Temp 1: ");
  lcd.print(sensors.getTempCByIndex(0));
  //displaying second temp
  lcd.setCursor(0,1);
  lcd.print("Temp 2: ");
  lcd.print(int(Thermistor(analogRead(A0))));
  
  // set the cursor to column 0, line 1
  // (note: line 2 is the third row, since counting begins with 0):
  lcd.setCursor(0, 2);
  // print the number of seconds since reset:
  lcd.print("Status 1: ");
  if (latch1 == 0)
    lcd.print("OFF");
  else
    lcd.print(" ON");
  //Set the cursor to the next row and print the status of button 2
  lcd.setCursor(0, 3);
  lcd.print("Status 2: ");
  if(latch2 == 0)
    lcd.print("OFF");
  else
    lcd.print(" ON");
  
  //lcd.print(millis() / 1000);
  lcd.setCursor(0, 0);
  //switch relay 1 if the button is pressed
  relaySwitch(relay1Pin, button1Pin, latch1);
  //switch relay 2 if the button is pressed
  relaySwitch(relay2Pin, button2Pin, latch2);
  
}


//FUNCTIONS

//function to swtich a relay
void relaySwitch(int relayPin, int buttonPin, int &latch){
  int buttonState = 0;         // variable for reading the pushbutton status

  //RELAY SWITCHING
   // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);
  //Update the latch
  if(buttonState == LOW)
    {
      if(latch == 0)
        latch = 1;
      else
        latch = 0;
    }
   else
   {
      //IGNORE
   }
   //If the latch is high, switch the relay on
   if (latch == 1)
    {
      digitalWrite(relayPin, HIGH);
    }
    //If the latch is low, switch the relay off
    else //(latch == 0)
    {
      digitalWrite(relayPin, LOW);
    }
}

// THERMISTOR
double Thermistor(int RawADC) {
 double Temp;
 Temp = log(10000.0*((1024.0/RawADC-1))); 
//         =log(10000.0/(1024.0/RawADC-1)) // for pull-up configuration
 Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp ))* Temp );
 Temp = Temp - 273.15;            // Convert Kelvin to Celcius
 //Temp = (Temp * 9.0)/ 5.0 + 32.0; // Convert Celcius to Fahrenheit
 return Temp;
}


