const int buttonPin = 0;     // the number of the pushbutton pin
const int ledPin =  13;      // the number of the LED pin
int latch = 0;
int buttonState = 0;         // variable for reading the pushbutton status

void setup() {
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);
  digitalWrite(ledPin, LOW);
  Serial.begin(9600);
} 

void loop() {
  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);
  if(buttonState == LOW)
    {
      delay(500);
      if(latch == 0)
        latch = 1;
      else
        latch = 0;
    }
   else
   {
      //IGNORE
   }
   if (latch == 1)
    {
      digitalWrite(ledPin, HIGH);
    }
    else //(latch == 0)
    {
      digitalWrite(ledPin, LOW);
    }
}
