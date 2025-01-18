

#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const int trigPin = 8;
const int echoPin = 7;
const int soundPin = 9;
const int potentiometerPin = 15;
 
void setup() { 

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(soundPin, OUTPUT);
  pinMode(potentiometerPin, INPUT);
// Wybór rodzaju wyświetlacza  - 16x2
lcd.begin(16, 2); 
//Przesłanie do wyświetlania łańcucha znaków hello, world!
lcd.print("hello, world!");
Serial.begin(9600);

}
 
void loop(){ 
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the duration of the high pulse on the Echo pin
  long duration = pulseIn(echoPin, HIGH);

  // Convert duration to distance in cm
  float distance = duration * 0.0343 / 2;
  float potentiometerVoltage = analogRead(potentiometerPin) * 5.0 / 1023.0;
  Serial.print(potentiometerVoltage);
  Serial.print(" voltage\n");
  float threshold = 200 * potentiometerVoltage / 5;
  Serial.print(threshold);
  Serial.print(" threshold\n");
  if (distance < threshold){
    digitalWrite(soundPin, HIGH);
  }
  else {
    digitalWrite(soundPin, LOW);
  }
  char str[10];
  dtostrf(distance, 6, 2, str);
  lcd.clear();
  lcd.print(str);

  delay(2000);
}