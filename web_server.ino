#define U_PART U_FS

void web_server_setup(){
  Serial.println(F("[Web server] Web server initialization"));
  
  web_server.on("/", HTTP_GET, handle_homepage);
  
  web_server.on("/update", HTTP_GET, handle_update_form);
  web_server.on("/update", HTTP_POST,
    [](AsyncWebServerRequest *request) {},
    [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data,
                  size_t len, bool final) {handleDoUpdate(request, filename, index, data, len, final);}
  );

  web_server.onNotFound(handle_not_found);
  web_server.begin();
}

void handle_homepage(AsyncWebServerRequest *request) {
  String html = apply_html_template(get_homepage());
    request->send(200, "text/html", html);
}


void handle_not_found(AsyncWebServerRequest *request) {
  request->send(404, "text/html", "Not found");
}

void handle_update_form(AsyncWebServerRequest *request){
  String html = apply_html_template(firmware_update_form);
  request->send(200, "text/html", html);
}

void handleDoUpdate(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (!index){
    Serial.println("Update");
    size_t content_len = request->contentLength();
    // if filename includes spiffs, update the spiffs partition
    int cmd = (filename.indexOf("spiffs") > -1) ? U_PART : U_FLASH;
    Update.runAsync(true);
    if (!Update.begin(content_len, cmd)) {
      Update.printError(Serial);
    }
  }

  if (Update.write(data, len) != len) {
    Update.printError(Serial);
  } else {
    Serial.printf("Progress: %d%%\n", (Update.progress()*100)/Update.size());
  }

  if (final) {
    AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Please wait while the device reboots");
    response->addHeader("Refresh", "20");  
    response->addHeader("Location", "/");
    request->send(response);
    if (!Update.end(true)){
      Update.printError(Serial);
    } else {
      Serial.println("Update complete");
      Serial.flush();
      ESP.restart();
    }
  }
}
