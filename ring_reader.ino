/*
 * Board type: Generic ESP8266
 */

// Libraries
#include <SoftwareSerial.h>
#include "SSD1306.h"

// Import fonts and images for the display
#include "font.h"
#include "images.h"

// IO
#define RX_PIN D5
#define TX_PIN D6
#define SDA_PIN D2
#define SCL_PIN D1
#define BUZZER_PIN D8


// Display parameters
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define DISPLAY_INVERSION_PERIOD 60000
#define DISPLAY_LOCK_STATE_TIME 5000

// reader parameters
#define COOLDOWN_DURATION 3000
#define CODE_ADDRESS 0x0b


// Misc
#define BUZZER_VOLUME 25

typedef struct Response {
  byte status = 0xff;
  byte length = 0;
  char data[0xff]; // This is a buffer
} Response;

// Declaring objects
SSD1306 display(0x3c, SDA_PIN, SCL_PIN);
SoftwareSerial swSer;

// Global variables
long cooldown_start_time = -COOLDOWN_DURATION;
char code[5] = { 0x69, 0x67, 0x91, 0x74, 0x76 };

boolean locked = false;
boolean displaying_lock_state = false;
long displaying_lock_state_start_time;

void setup() {
  delay(100);
	Serial.begin(115200);
  Serial.println("");
  Serial.println("Start");

  swSer.begin(9600, SWSERIAL_8N1, RX_PIN, TX_PIN, false, 256);

  buzzer_init();

  display_setup();
  display_image(logo);
}

void loop() {

  invert_display_periodically();

  manage_display_lock_state();


  int result = compare_em4100(code);
  if(result != -1){
    
    if(millis() - cooldown_start_time > COOLDOWN_DURATION){
      cooldown_start_time = millis();
      
      if(result == 0) {
        display_check();
        buzzer_play_success();

        // Demo
        locked = !locked;
        display_lock_state();
        
      }
      else if(result == 1) {
        display_cross();
        buzzer_play_error();
        delay(1000);
        display_lock_state();
      }
      
    }
  }
    
}
