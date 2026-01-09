# Simple version of MP3 Music Player

## Sample sketch

```c++
#include "CYD_MP3Player.h"

CYD_MP3Player player;

void setup() {
  Serial.begin(115200);
  while (millis() < 1000);

  audioInit();                  // Create a task to play audio file

  player.begin();               // Mount the SD card
  player.SetVolume(8);          // Set the volume
  player.ScanFileList("/", 3);  // Scan up to the 3 level and create a playlist
  player.SortFileList(true);    // Sort the playlist in ascending order
}

void loop() {
  player.AutoPlay();

  if (Serial.available() > 0) {
    int v = Serial.readStringUntil('\n').toInt();
    player.SetVolume((uint8_t)constrain(v, 0, 21));
  }
}
```
