
int out1 = 12;
int out2 = 13;
const int trigPin = 9;
const int echoPin = 8;
long duration;
int distance;

void setup() 
{
  // put your setup code here, to run once:
  
 pinMode(out1, OUTPUT);
 pinMode(out2, OUTPUT);
 pinMode(trigPin, OUTPUT); 
pinMode(echoPin, INPUT); 
Serial.begin(9600);
}

void loop() 
{
  // put your main code here, to run repeatedly:
  
/*digitalWrite(out1, HIGH);
delay(1000);
digitalWrite(out1, LOW);
digitalWrite(out2, HIGH);
delay(1000);
digitalWrite(out2, LOW);*/

digitalWrite(trigPin, LOW);
delayMicroseconds(2);
// Sets the trigPin on HIGH state for 10 micro seconds
digitalWrite(trigPin, HIGH);
delayMicroseconds(10);
digitalWrite(trigPin, LOW);
// Reads the echoPin, returns the sound wave travel time in microseconds
duration = pulseIn(echoPin, HIGH);
// Calculating the distance
distance= duration*0.034/2;
// Prints the distance on the Serial Monitor
Serial.print("Distance: ");
Serial.println(distance);

if (distance > 20)
{
digitalWrite(out2, HIGH);
digitalWrite(out1, HIGH);
//delay(1000);
//digitalWrite(out1, LOW);
//delay(1000);
}

if (distance <= 20 && distance > 10)
{
digitalWrite(out2, LOW);
digitalWrite(out1, HIGH);
delay(1000);
digitalWrite(out1, LOW);
delay(1000);
}

if (distance <= 10 && distance >7)
{
digitalWrite(out1, LOW);
digitalWrite(out2, HIGH);
delay(1000);
digitalWrite(out2, LOW);
delay(1000);
}

if (distance < 7)
{
digitalWrite(out1, LOW);
digitalWrite(out2, HIGH);
delay(100);
digitalWrite(out2, LOW);
delay(100);
}
}
