#include "CYD28_audio.h"

void setup() {
  Serial.begin(115200);
  while (millis() < 1000);

  if (!SD.begin()) {
    Serial.println("Cannot begin SD.");
    while (1);
  }

  audioInit();  // Create a task to play audio file
  delay(10);    // Wait until the task on Core 1 is ready to receive a command
  audioConnecttoSD("/sample.mp3");
}

void loop() {
  if (Serial.available() > 0) {
    int v = Serial.readStringUntil('\n').toInt();
    audioSetVolume((uint8_t)constrain(v, 0, 21));
  }
}

// optional
void audio_info(const char *info) {
  Serial.print("info        ");
  Serial.println(info);
}
void audio_id3data(const char *info) {  //id3 metadata
  Serial.print("id3data     ");
  Serial.println(info);
}
void audio_eof_mp3(const char *info) {  //end of file
  Serial.print("eof_mp3     ");
  Serial.println(info);
}
void audio_showstation(const char *info) {
  Serial.print("station     ");
  Serial.println(info);
}
void audio_showstreamtitle(const char *info) {
  Serial.print("streamtitle ");
  Serial.println(info);
}
void audio_bitrate(const char *info) {
  Serial.print("bitrate     ");
  Serial.println(info);
}
void audio_commercial(const char *info) {  //duration in sec
  Serial.print("commercial  ");
  Serial.println(info);
}
void audio_icyurl(const char *info) {  //homepage
  Serial.print("icyurl      ");
  Serial.println(info);
}
void audio_lasthost(const char *info) {  //stream URL played
  Serial.print("lasthost    ");
  Serial.println(info);
}