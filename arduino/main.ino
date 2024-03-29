#include <Stepper.h>
#include <Servo.h>
#include <SoftwareSerial.h>
#include <AFMotor.h>
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_PWMServoDriver.h"

#define PIN_COF (5)
#define PIN_SERVO (6)
#define strokeMax (100)
// TODO: Change DISPENSE_OFFSET to be correct value for between the shots
#define DISPENSE_OFFSET (500)
#define MAX_INPUT_LENGTH (6)

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
Adafruit_StepperMotor *beltServo = AFMS.getStepper(200, 2);

int position = 0;
boolean chkString = false;
String inputString = ""; // a string to hold incoming data
boolean stringComplete = false;
int vars[MAX_INPUT_LENGTH] = {};

Servo shotServo;


void setup()
{
    // start serial port at 9600 bps and wait for port to open:
    Serial.begin(9600);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for Leonardo only
    }
    AFMS.begin();  // create with the default frequency 1.6KHz
    inputString.reserve(200); // Hold 200 bytes for input string
    pinMode(PIN_COF,OUTPUT); // pin for controlling coffee relay
    pinMode(2, INPUT);     // digital sensor is on digital pin 2
    shotServo.attach(PIN_SERVO); // Setup shotservo on defined pin
    beltServo->setSpeed(10);  // 10 rpm 
    establishContact();    // send a byte to establish contact until receiver responds
}

void loop()
{
    // if we get a valid byte, read analog ins:
    while (stringComplete) {
        Serial.println(inputString.length());
        if(inputString.length() == MAX_INPUT_LENGTH){
            parseInput(inputString);
            delay(30000);
            dispenseCoffee();
            beltServo->step(DISPENSE_OFFSET, BACKWARD, SINGLE);
            for(int i = 1; i < MAX_INPUT_LENGTH; i++){
                // Move servo into position
                beltServo->step(i*DISPENSE_OFFSET, BACKWARD, SINGLE);
                // If requested serve shot
                if(vars[i] == '1'){
                    Serial.println(vars[i]);
                    dispenseShot();
                }
                
            }
            // Reset belt to starting position
            beltServo->step(MAX_INPUT_LENGTH*DISPENSE_OFFSET, FORWARD,SINGLE);
            Serial.println(inputString);
        }
        inputString = "";
        chkString = false;
        stringComplete = false;
    }

}

void establishContact() {
    while (Serial.available() <= 0) {
        Serial.println("Waiting..."); // send an initial string
        delay(300);
    }
}

void parseInput(String s) {
    int sum = 0;
    for(int i = 0; i < (s.length() -1); i++){
        if(isDigit(s.charAt(i))){
            vars[i] = (s.charAt(i) - '0');
            sum += vars[i];
        }
    }
    int chkInt = s.charAt(s.length()) - '0';
    if(sum = chkInt){
        chkString = true;
    }
}

void serialEvent() {
    while (Serial.available()) {
        if(inputString.length() > 50){
            inputString = ""; // If size getting to large reset
        }
        // get the new byte:
        char inChar = (char)Serial.read();
        // add it to the inputString:
        if (inChar == '\n') {
            stringComplete = true;
        } else {
            inputString += inChar;
        }
    }
}

void dispenseShot(){
    SetStrokePerc(60);
    delay(3000);
    SetStrokePerc(1);
    delay(3000);
}
void dispenseCoffee(){
  digitalWrite(PIN_COF, HIGH);
  delay(10000);
  digitalWrite(PIN_COF, LOW);
  delay(5000);
}

void SetStrokePerc(float strokePercentage)
{
    if ( strokePercentage >= 1.0 && strokePercentage <= 99.0 )
    {
        int usec = 1000 + strokePercentage * ( 2000 - 1000 ) / 100.0 ;
        shotServo.writeMicroseconds( usec );
    }
}
