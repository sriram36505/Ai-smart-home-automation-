#define BLYNK_TEMPLATE_ID "TMPL3cRz-c2ux"
#define BLYNK_TEMPLATE_NAME "Smart House Automation Smart Keypad Locking"
#include <Arduino.h>
#undef HIGH
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>  // Blynk library for ESP8266
#define HIGH 0x1

// Blynk authentication token
char auth[] = "2ceAa-mc3MUKAnJutfbXfxa8DUqgGZ1l";  // Replace with your Blynk Auth Token

// Your WiFi credentials
char ssid[] = "G";       // Replace with your WiFi SSID
char pass[] = "green apples";     // Replace with your WiFi password

// Define servo and buzzer pins
#define SERVO_PIN D8   // Adjust pin as per your setup
#define BUZZER_PIN D9  // Adjust pin as per your setup

// Define LCD parameters
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Set LCD address to 0x27 for a 16 chars and 2 line display

// Define keypad parameters
const byte ROWS = 4;  // Four rows
const byte COLS = 3;  // Three columns
char keys[ROWS][COLS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}
};

// Connect to the row and column pins
byte rowPins[ROWS] = {D0, D1, D3, D4};  // Adjust row pins as per your wiring
byte colPins[COLS] = {D5, D6, D7};  // Adjust column pins as per your wiring

// Initialize the keypad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Variables
Servo servo;
String inputCode = "";
const String unlockCode = "1234";  // Example unlock code
int failedAttempts = 0;  // Track number of failed attempts

void setup() {
    // Initialize components
    Wire.begin(4, 5);  // Initialize I2C with SDA on GPIO4 and SCL on GPIO5
    lcd.begin(16, 2);  // Initialize the LCD for 16x2 characters
    lcd.backlight();   // Turn on the LCD backlight

    Blynk.begin(auth, ssid, pass);  // Initialize Blynk
    servo.attach(SERVO_PIN);  // Attach the servo
    pinMode(BUZZER_PIN, OUTPUT);  // Set buzzer as output
    servo.write(0);  // Lock door initially (servo position 0)

    // Display welcome message
    lcd.setCursor(0, 0);  
    lcd.print("Welcome to");
    lcd.setCursor(0, 1);
    lcd.print("Keypad Locking");
    delay(2000);  // Show for 2 seconds

    // Clear the LCD for user input
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter code:");
}

void unlockDoor() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Door Unlocked!");  // Display unlock message
    Blynk.logEvent("door_unlocked"); 
    servo.write(170);  // Move servo to unlock position
    digitalWrite(BUZZER_PIN, HIGH);  // Turn buzzer on
    delay(900);
    digitalWrite(BUZZER_PIN, LOW);  // Turn buzzer off
    failedAttempts = 0;  // Reset failed attempts
    delay(2000);  // Keep the unlock message for 2 seconds
    lcd.clear();  // Clear after showing unlock message
    lcd.setCursor(0, 0);
    lcd.print("Enter code:");  // Reset display for new input
}

void lockDoor() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Door Locked!");  // Display lock message
    servo.write(0);  // Move servo back to lock position
    delay(2000);  // Keep the lock message for 2 seconds
    lcd.clear();  // Clear after showing lock message
    lcd.setCursor(0, 0);
    lcd.print("Enter code:");  // Reset display for new input
}

void failedAttempt() {
    Serial.println("\nWrong Code!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Worng Code!");  // Print in a new line for wrong code
    failedAttempts++;
    // Emit error signal on buzzer (long beep, pause, then two short beeps)
    digitalWrite(BUZZER_PIN, HIGH);
    delay(800);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);

    if (failedAttempts >= 3) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("3 Failed Attempts!");  // Display failed attempts message
        lcd.setCursor(0, 1);
        lcd.print("Sending Noti...");
        Blynk.logEvent("3_wrong_attempts", "3 Failed Attempts! Someone is trying to unlock the door.");  // Log failed attempts event
        failedAttempts = 0;  // Reset failed attempts after notification
    }
    delay(2000);  // Keep the wrong code message for 2 seconds
    lcd.clear();  // Clear after showing wrong code message
    lcd.setCursor(0, 0);
    lcd.print("Enter code:");  // Reset display for new input
}

void loop() {
    Blynk.run();  // Run Blynk in the loop

    char key = keypad.getKey();  // Get keypad input

    if (key) {
        if (key == '#') {  // '#' indicates to check the code
            if (inputCode == unlockCode) {
                unlockDoor();  // Correct code
            } else {
                failedAttempt();  // Handle wrong code
            }
            inputCode = "";  // Reset input after checking
        } else if (key == '*') {  // '' indicates locking
            lockDoor();  // Lock the door
            inputCode = "";  // Clear input
        } else {
            inputCode += key;  // Append key to the input code
            lcd.setCursor(0, 1);
            lcd.print("Code: ");
            lcd.print(inputCode);  // Display the current input code
        }
    }
}