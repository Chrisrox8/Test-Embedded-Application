#include <LiquidCrystal.h>

int tempSensor = 9;
//int LCD = 1;

//pinMode(LCD, OUTPUT);
//pinMode(LCD, OUTPUT);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(digitalRead(tempSensor));
  delay(500);
 //int tempVal = analogRead(tempSensor);
 
}
