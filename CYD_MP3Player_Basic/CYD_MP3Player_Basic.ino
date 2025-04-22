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
  player.SetVolume(10);
}

void loop() {
  player.AutoPlay();

  if (Serial.available() > 0) {
    int v = Serial.readStringUntil('\n').toInt();
    player.SetVolume((uint8_t)constrain(v, 0, 21));
  }
}