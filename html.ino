/*
 * This file contains the HTML for the web pages
 */

// Head partials
// Style
const String styles = ""
  "<style>"
  "body { font-family: Arial, sans-serif; }"
  "nav { display: flex; }"
  "nav a { margin: 0.25em; }"
  "nav a::first-child {margin-left: 0; }"
  "label { display: inline-block; width: 250px; }"
  "table { border-collapse: collapse; }"
  "/*tr:not(:last-child) { border-bottom: 1px solid #dddddd; }*/"
  "th { text-align: left; width: 200px; }"
  "</style>";

String get_html_head() {
  return ""
    "<head>"
    "<title>" + get_device_name() + "</title>"
    + styles +
    "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
    "</head>";
}

// Body partials
String get_html_header(){
  return ""
    "<header>"
    "  <h1>" + get_device_name() + "</h1>"
    "</header>";
}

const String nav = ""
  "<nav>"
    "<a href='/'>Info</a>"
    "<a href='/settings'>Settings</a>"
    "<a href='/update'>Firmware update</a>"
  "</nav>";

// Main contents
String get_homepage(){
  return ""
    "<h2>Info</h2>"
    "<h3>Device</h3>"
    "<table>"
      "<tr><th>Device type</th><td>" + String(DEVICE_TYPE) + "</td></tr>"
      "<tr><th>Device name</th><td>" + get_device_name() + "</td></tr>"
      "<tr><th>Firmware version</th><td>"+ String(DEVICE_FIRMWARE_VERSION) +"</td></tr>"
    "</table>"
    "<h3>WiFi</h3>"
    "<table>"
      "<tr><th>SSID</th><td>" + String(WIFI_SSID) + "</td></tr>"
      "<tr><th>IP</th><td>" + WiFi.localIP().toString() + "</td></tr>"
      "<tr><th>RSSI</th><td>" + String(WiFi.RSSI()) + "</td></tr>"
    "</table>"
    "<h3>MQTT</h3>"
    "<table>"
      "<tr><th>Broker URL</th><td>" + MQTT_BROKER_ADDRESS.toString() + "</td></tr>"
      "<tr><th>Username</th><td>" + String(MQTT_USERNAME) + "</td></tr>"
      "<tr><th>Client ID</th><td>" + get_device_name() + "</td></tr>"
      "<tr><th>Connection</th><td>" + (mqtt_connected() ? "connected" : "disconnected") + "</td></tr>"
      "<tr><th>Lock command topic</th><td>" + String(MQTT_LOCK_COMMAND_TOPIC)+ "</td></tr>"
      "<tr><th>Lock statustopic</th><td>" + String(MQTT_LOCK_STATUS_TOPIC) + "</td></tr>"
    "</table>"
    ;
}
  


String firmware_update_form = ""
  "<h2>Firmware update</h2>"
    "<form method='POST' action='/update' enctype='multipart/form-data'>"
    "<input type='file' name='update'>"
    "<input type='submit' value='Update'>"
  "</form>";


String firmware_update_success = ""
  "<h2>Firmware update</h2>"
  "<p>Upload successful, rebooting...</p>";

String firmware_update_failure = ""
  "<h2>Firmware update</h2>"
  "<p>Upload failed</p>";
  
// Assembling the page
String apply_html_template(String main_content){
  return ""
    "<html>"
    + get_html_head() 
    + "<body>"
    + get_html_header()
    + nav 
    + "<main>"
    + main_content
    + "</main>"
    "</body>"
    "</html>";
}
