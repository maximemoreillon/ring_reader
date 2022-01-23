String get_device_name(){
  String chip_id = String(ESP.getChipId(), HEX);
  return String(DEVICE_TYPE) + "-" + chip_id;
}
