void MQTT_setup(){
  Serial.println(F("[MQTT] MQTT setup"));
  
  MQTT_client.setServer(MQTT_BROKER_ADDRESS, MQTT_PORT);
  MQTT_client.setCallback(MQTT_message_callback);
}

boolean mqtt_connected(){
  return MQTT_client.connected();
}

void MQTT_connection_manager(){

  static boolean last_connection_state = false;
  
  static long last_connection_attempt;
  
  if(mqtt_connected() != last_connection_state) {
    last_connection_state = mqtt_connected();

    if(mqtt_connected()){
      // Changed from disconnected to connected
      Serial.println(F("[MQTT] Connected"));
      
      
      Serial.println(F("[MQTT] Subscribing to topics"));
      MQTT_client.subscribe(MQTT_LOCK_STATUS_TOPIC);
    }
    else {
      // Changed from connected to disconnected
      Serial.print(F("[MQTT] Disconnected: "));
      Serial.println(MQTT_client.state());
      if(wifi_connected()) display_image(mqtt_disconnected);
    }
  }

  // Kind of similar to the pubsubclient example, one connection attempt every 5 seconds
  if(!mqtt_connected()){
    if(millis() - last_connection_attempt > MQTT_RECONNECT_PERIOD){
      last_connection_attempt = millis();

      // No need to do anything if not connected to WiFi
      if(!wifi_connected()) return;
      
      Serial.println("[MQTT] Connecting");
      
      MQTT_client.connect(
        get_device_name().c_str(),
        MQTT_USERNAME, 
        MQTT_PASSWORD
      );
    }
  }
  
}



void MQTT_message_callback(char* topic, byte* payload, unsigned int len) {

  // listening to lock states

  // Debugging
  Serial.print(F("[MQTT] message received on "));
  Serial.print(topic);
  Serial.print(F(", payload: "));
  for (int i = 0; i < len; i++) Serial.print((char)payload[i]);
  Serial.println();

  static boolean locked_previous = false;

  // Create a JSON object to hold the message
  // Note: size is limited by MQTT library
  StaticJsonDocument<MQTT_MAX_PACKET_SIZE> inbound_JSON_message;

  // Copy the message into the JSON object
  deserializeJson(inbound_JSON_message, payload);

  if(inbound_JSON_message.containsKey("state")){

    Serial.println("[MQTTT] Payload is JSON with state");

    // Check what the command is and act accordingly
    // Use strdup so as to use strlwr later on
    char* command = strdup(inbound_JSON_message["state"]);

    if( strcmp(strlwr(command), "locked") == 0 ) {
      locked = true;
    }
    else if( strcmp(strlwr(command), "unlocked") == 0 ) {
      locked = false;
    }

    free(command);

  }


  if(locked != locked_previous){
    locked_previous = locked;
    display_lock_state();
    buzzer_lock_state_changed();
  }
  
}


void MQTT_publish_toggle(){
  //Send the payload

  StaticJsonDocument<MQTT_MAX_PACKET_SIZE> outbound_JSON_message;
  
  if(locked) {
    outbound_JSON_message["state"] = "unlocked";
  }
  else {
    outbound_JSON_message["state"] = "locked";
  }

  char mqtt_payload[MQTT_MAX_PACKET_SIZE];
  serializeJson(outbound_JSON_message, mqtt_payload, sizeof(mqtt_payload));

  MQTT_client.publish(MQTT_LOCK_COMMAND_TOPIC, mqtt_payload, MQTT_RETAIN);
  
}

void MQTT_publish_location(){
  // Inform the home automation system that the user is outside

  StaticJsonDocument<MQTT_MAX_PACKET_SIZE> outbound_JSON_message;
  
  if(!locked) {
    outbound_JSON_message["location"] = "out";
  }

  char mqtt_payload[MQTT_MAX_PACKET_SIZE];
  serializeJson(outbound_JSON_message, mqtt_payload, sizeof(mqtt_payload));

  MQTT_client.publish(MQTT_LOCATION_TOPIC, mqtt_payload, MQTT_RETAIN);
  
}
