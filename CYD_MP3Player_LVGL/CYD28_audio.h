#ifndef _CYD28_AUDIO_H_
#define _CYD28_AUDIO_H_

#include "src/CYD_Audio/CYD_Audio.h"

/**
 * @brief Audio taks commands
 */
typedef enum : uint8_t
{
  IS_PLAYING,
  SET_VOLUME,
  GET_VOLUME,
  GET_RMS,
  GET_VU,
  GET_DURATION,
  GET_ELAPSED,
  SET_ELAPSED,
  CONNECTTOHOST,
  CONNECTTOSPEECH,
  CONNECTTOSD,
  AUDIO_STOP,
  PAUSE_RESUME,
} audioCmd_t;

/**
 * @brief Audio task queue message
 */
typedef struct 
{
  audioCmd_t cmd;
  const char *txt1;
  const char *txt2;
  uint32_t value;
  uint32_t ret;
} audioMessage_t;

extern CYD_Audio audio;

void audioInit();
audioMessage_t transmitReceive(audioMessage_t msg);

bool audioIsPlaying(void);
void audioSetVolume(uint8_t vol);
uint8_t audioGetVolumePerCent();
uint16_t audioGetVU();  // for USE_I2S_DAC
uint32_t audioGetRMS(); // for internal DAC
uint32_t audioGetDuration();
uint32_t audioGetElapsedTime();
bool audioSetElapsedTime(uint32_t time);
bool audioConnecttohost(const char *host);
bool audioConnecttoSD(const char *filename);
bool audioConnecttoSpeech(const char *host, const char *lang);
void audioStopSong();
void audioPauseResume();
void setVuMeters(uint32_t vuRL);

#endif // _AUDIO_H_