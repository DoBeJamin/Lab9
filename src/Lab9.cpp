/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "c:/Users/benja/OneDrive/Desktop/visualStudioCode/summerIOT/Lab9/src/Lab9.ino"
#include <env.h>
#include <Wire.h>
#include <cstring>

#include "blynk.h"
#include "oled-wing-adafruit.h"
#include "SparkFun_VCNL4040_Arduino_Library.h"

void setup();
void release_lockdown();
void loop();
void format_display ();
#line 9 "c:/Users/benja/OneDrive/Desktop/visualStudioCode/summerIOT/Lab9/src/Lab9.ino"
SYSTEM_THREAD(ENABLED);

//component objects
VCNL4040 proximitySensor;
OledWingAdafruit display;

//name pins
#define LED1 D5
#define LED2 D6
#define LED3 D7

#define Button D8

#define Potentiometer A0

#define Temperature A5

//internal variables
bool setup_mode = true;
bool button_pressed = false;
bool lock_down = false;

int light_level_flags[2] = {0,0};
int light_level_flag_index = 0;
int ambient_value;

int current_range;
int old_range;

int reading;
double voltage;
double temperature_celcius;
double temperature_fahrenheit;

Timer timer(10000, release_lockdown);

void setup() {

  Serial.begin(9600);

  //VNCL setup
  Wire.begin();
  //turns on sensor
  proximitySensor.begin(); 
  proximitySensor.powerOnAmbient();
  proximitySensor.enableWhiteChannel();

  //led setup
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  //button setup
  pinMode(Button, INPUT);

  //potentiometer
  pinMode(Potentiometer, INPUT);

  //temperature setup
  pinMode(Temperature, INPUT);

  //oled setup
  display.setup();

  //blynk setup
  Blynk.begin(BLYNK_AUTH_TOKEN);

}

BLYNK_WRITE(V3) {
  //displays lock down method and turns the lockdown variable on
  lock_down = true;
  format_display();
  display.println("LOCK DOWN!");
  display.display();
  timer.start();

}

void release_lockdown() {
  lock_down = false;
}



void loop() {
  //maintain display and blynk
  display.loop();
  Blynk.run();

  //stops everything from happening unless completely neccesary if lock down mode is on
  if (!lock_down) {

    //activates setup mode
    if (display.pressedA()) {
      setup_mode = true;
    }
    
    //calculate temperatures
    reading = analogRead(Temperature);
    voltage = (reading * 3.3) / 4095.0;
    temperature_celcius = (voltage - 0.5) * 100;
    temperature_fahrenheit = (temperature_celcius * 9.0 / 5.0) + 32.0;

    //sends temps to blynk
      Blynk.virtualWrite(V0, temperature_fahrenheit);
      Blynk.virtualWrite(V1, temperature_celcius);

    // only changes the light_level_flag_index once when the button is pressed
    if (digitalRead(Button))
    {
      button_pressed = true;
    }
    else if (button_pressed)
    {
      light_level_flag_index++;
      if (light_level_flag_index == 2)
      {
        setup_mode = false;
        light_level_flag_index = 0;
      }
      button_pressed = false;
    }

    if (setup_mode) {
      //displays information for setting values and allows the user to set a value
      light_level_flags[light_level_flag_index] = analogRead(Potentiometer);

      format_display();
      display.print("Choose limit ");
      display.println(light_level_flag_index+1);
      display.println("range: 0,4095 ");
      display.print(light_level_flags[0]);
      display.print(", ");
      display.println(light_level_flags[1]);

      display.display();

    } else {

      //format screen and text
      format_display();
      display.println("TEMPERATURE");
      display.print(temperature_celcius);
      display.print("C : ");
      display.print(temperature_fahrenheit);
      display.print("F");

      //display screen
      display.display();

        //sends alerts
      if (current_range != old_range) {
        Blynk.logEvent("rangechange", current_range); 
      }
      old_range = current_range;

    }
  }


  //collect current proximity information
  ambient_value = proximitySensor.getAmbient();

  //display proximity on LED's
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  if  (ambient_value < min(light_level_flags[0], light_level_flags[1])) {
    digitalWrite(LED1, HIGH);
    current_range = 1;
  } else if (ambient_value < max(light_level_flags[0], light_level_flags[1])) {
    digitalWrite(LED2, HIGH);
    current_range = 2;
  } else {
    digitalWrite(LED3, HIGH);
    current_range = 3;
  }



}

void format_display () {
  //sets up basic formatting for when using the display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE); 
  display.setCursor(0,0);
}