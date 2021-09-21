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

  // Debugging
  Serial.print(F("[MQTT] message received on "));
  Serial.print(topic);
  Serial.print(F(", payload: "));
  for (int i = 0; i < len; i++) Serial.print((char)payload[i]);
  Serial.println();

  
  if(strncmp((char*)payload, "UNLOCKED", len) == 0){
    locked = false;
    display_lock_state();
  }
  else if(strncmp((char*)payload, "LOCKED",len) == 0){
    locked = true;
    display_lock_state();
  }
  
}


void MQTT_publish_toggle(){
  //Send the payload
  if(locked) {
    MQTT_client.publish(MQTT_LOCK_COMMAND_TOPIC, "UNLOCK", MQTT_RETAIN);
  }
  else {
    MQTT_client.publish(MQTT_LOCK_COMMAND_TOPIC, "LOCK", MQTT_RETAIN);
  }
  
}
