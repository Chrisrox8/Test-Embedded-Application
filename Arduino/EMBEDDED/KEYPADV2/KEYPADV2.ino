#include "Keypad.h"

 
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char keys[ROWS][COLS] =
 {{'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}};
byte rowPins[ROWS] = {
  5, 4, 3, 2}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {
  9, 8, 7, 6}; //connect to the column pinouts of the keypad
int count=0;
 
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
 
void setup()
{
  Serial.begin(9600);
  Serial.begin(16, 2);
  Serial.print("Keypad test!");  
  delay(1000);
  Serial.print("Initialize: ");
  
}
 
void loop()
{
  char key = keypad.getKey();
  if (key != NO_KEY)
  {
    Serial.print(key);
    Serial.print(key);
    count++;
    if (count==17)
    {
      count=0;
    }
  }
}
