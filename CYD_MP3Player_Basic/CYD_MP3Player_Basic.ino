// All necessary files are included
#include "CYD_MP3Player.h"
CYD_MP3Player player;

/*--------------------------------------------------------------------------------
 * Setup and Loop
 *--------------------------------------------------------------------------------*/
void setup() {
  Serial.begin(115200);
  while (millis() < 1000);

  audioInit();

  player.begin();
  player.ScanFileList("/", 3);
  player.SortFileList(true);
  player.SetVolume(8);
}

void loop() {
  player.AutoPlay();

  if (Serial.available() > 0) {
    int v = Serial.readStringUntil('\n').toInt();
    player.SetVolume((uint8_t)constrain(v, 0, 21));
  }
}

/*--------------------------------------------------------------------------------
 * Optional functions for audio-I2S
 *--------------------------------------------------------------------------------*/
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

  // play next
  player.PlayNext(false);
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