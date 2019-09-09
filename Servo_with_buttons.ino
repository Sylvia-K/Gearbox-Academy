#include <Servo.h>
Servo servo1;

const int buttonPin1 = 2;// the number of the pushbutton pin
const int buttonPin2 = 3;
int leftPressed = 0;
int rightPressed = 0;

int redPin= 5;
int greenPin = 6;
int bluePin = 9;

int pos = 20;

void setup() 
{
  // put your setup code here, to run once:
  servo1.attach(11); 
  pinMode(buttonPin1, INPUT);
  pinMode(buttonPin2, INPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
}

void loop() 
{
  leftPressed = digitalRead(buttonPin2);
  rightPressed = digitalRead(buttonPin1);
  setColor(255, 40, 60);
  
  if (leftPressed == HIGH)
  {
      for (pos = 20; pos <= 160; pos += 3) 
      { // goes from 0 degrees to 180 degrees
        // in steps of 1 degree
        servo1.write(pos);              // tell servo to go to position in variable 'pos'
        delay(100);   // waits 15ms for the servo to reach the position
        setColor(255, 255, 255); // White Color
      }
  }
  
  if (rightPressed == HIGH)
  {
      for (pos = 180; pos >= 20; pos -= 3) 
      { // goes from 180 degrees to 0 degrees
        servo1.write(pos);              // tell servo to go to position in variable 'pos'
        delay(100);                       // waits 15ms for the servo to reach the position
        setColor(170, 58, 255);
      }
  }
}

void setColor(int redValue, int greenValue, int blueValue) 
{
  analogWrite(redPin, redValue);
  analogWrite(greenPin, greenValue);
  analogWrite(bluePin, blueValue);
}
  

  

