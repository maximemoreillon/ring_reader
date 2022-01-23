void buzzer_init() {
  pinMode(BUZZER_PIN, OUTPUT);
  
  digitalWrite(BUZZER_PIN, LOW);

  analogWrite(BUZZER_PIN, 0);


}


void tone_custom(int freq, int duration){
  analogWriteFreq(freq);
  analogWrite(BUZZER_PIN, BUZZER_VOLUME);
  delay(duration);
  analogWrite(BUZZER_PIN, 0);
}

void buzzer_play_success(){
  tone_custom(1800,100);
  tone_custom(2300,100);
  tone_custom(2500,500);
}

void buzzer_lock_state_changed(){
  tone_custom(2300,50);
  delay(50);
  tone_custom(2300,100);
}


void buzzer_play_error(){

  tone_custom(300,100);
  delay(100);
  tone_custom(300,500);
  
}
