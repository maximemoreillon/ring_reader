boolean wifi_connected(){
  return WiFi.status() == WL_CONNECTED;
}

void wifi_setup() {

  Serial.println(F("[WiFi] Wifi starting"));
  
  WiFi.hostname(get_device_name());
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void wifi_connection_manager(){
  // Checks for changes in connection status

  static boolean last_connection_state = false;

  if(wifi_connected() != last_connection_state) {
    last_connection_state = wifi_connected();

    if(wifi_connected()){
      Serial.print(F("[WIFI] Connected, IP: "));
      Serial.println(WiFi.localIP());
      display_wifi_connected();
    }
    else {
      Serial.println(F("[WIFI] Disconnected"));
    }
  }

}
