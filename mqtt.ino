void mqtt_publish_valid_tag(){
  String topic = iot_kernel.mqtt_base_topic + "/events";
  Serial.print("[MQTT] Publishing to ");
  Serial.println(topic);

  StaticJsonDocument<MQTT_MAX_PACKET_SIZE> outbound_JSON_message;
  
  outbound_JSON_message["tag"] = "valid";
  
  char mqtt_payload[MQTT_MAX_PACKET_SIZE];
  serializeJson(outbound_JSON_message, mqtt_payload, sizeof(mqtt_payload));

  iot_kernel.mqtt.publish(topic.c_str(), mqtt_payload, MQTT_RETAIN);
  
}
