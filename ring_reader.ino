
/*
 * Board type: ESP32 Dev module
 */
// Libraries
#include "IotKernel.h"
#include "SSD1306.h"

// Import fonts and images for the display
#include "font.h"
#include "images.h"

//Device info
#define DEVICE_TYPE "em4305"
#define DEVICE_FIRMWARE_VERSION "0.0.1"

// IO
#define RX_PIN 13
#define TX_PIN 16
#define SDA_PIN 5
#define SCL_PIN 4
#define BUZZER_PIN 25


// Display parameters
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define DISPLAY_INVERSION_PERIOD 60000
#define DISPLAY_LOCK_STATE_TIME 5000

// reader parameters
#define COOLDOWN_DURATION 3000
#define CODE_ADDRESS 0x0b

// MQTT parameters
#define MQTT_LOCK_COMMAND_TOPIC "lock/command"
#define MQTT_LOCK_STATUS_TOPIC "lock/status"
#define MQTT_EVENTS_TOPIC "/ring-reader/events"
#define MQTT_RETAIN true
#define MQTT_RECONNECT_PERIOD 1000


// Misc
#define BUZZER_CHANNEL 0
#define BUZZER_RESOLUTION 8

typedef struct Response {
  byte status = 0xff;
  byte length = 0;
  char data[0xff]; // This is a buffer
} Response;

// Declaring objects
SSD1306 display(0x3c, SDA_PIN, SCL_PIN);
IotKernel iot_kernel(DEVICE_TYPE,DEVICE_FIRMWARE_VERSION); 


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

  //  Serial for the EM4305 reader
  Serial1.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);

  buzzer_init();

  display_setup();
  display_image(logo);
  
  iot_kernel.init();

}

void loop() {

  iot_kernel.loop();

  invert_display_periodically();



  int result = compare_em4100(code);
  if(result != -1){
    
    if(millis() - cooldown_start_time > COOLDOWN_DURATION){
      cooldown_start_time = millis();
      
      if(result == 0) {
        display_check();
        buzzer_play_success();
        mqtt_publish_valid_tag();
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
