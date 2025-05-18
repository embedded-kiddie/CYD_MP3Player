/*--------------------------------------------------------------------------------
 * CYD_MP3Player class definition
 *--------------------------------------------------------------------------------*/
#ifndef _CYD_MP3PLAYER_H_
#define _CYD_MP3PLAYER_H_

#include "CYD28_audio.h"

/*--------------------------------------------------------------------------------
 * Possible values ​​for `SetVolume()`
 *--------------------------------------------------------------------------------*/
#define MP3_VOLUME_MIN  0
#define MP3_VOLUME_MAX  21

/*--------------------------------------------------------------------------------
 * File name and size for ScanFileList()
 *--------------------------------------------------------------------------------*/
#include <string.h>
#include <string>
#include <vector>
#include <random>
#include <exception>

typedef struct {
  std::string path;
  std::string title;
  std::string artist;
  std::string album;
  uint32_t    duration;
} PlayList_t;

/*--------------------------------------------------------------------------------
 * Definition of SPI file system for audio files
 * NOTE: uncomment the followings to use SdFat 
 *  "#define SDFATFS_USED" in CYD_Audio.h
 *  "#define USE_UTF8_LONG_NAMES 1" in SdFatConfig.h
 *--------------------------------------------------------------------------------*/
#define SD_CLOCK  25000000 // The maximum SD SPI clock of ESP32-2432S028 would be 24 MHz
#define SD_CS     SS

#if defined (SDFATFS_USED)  // defined in CYD_Audio.h
#define FS_DEV    SD_SDFAT  // defined in CYD_Audio.cpp
#define FS_CONFIG SD_CS, SD_CLOCK
#define BUF_SIZE  64
#elif defined (_SD_H_)
#define FS_DEV    SD
#define FS_CONFIG SD_CS, SPI, SD_CLOCK
#endif

class CYD_MP3Player {
private:
  uint32_t m_playNo = 0;
  fs::FS & m_fs = FS_DEV;
  std::vector<PlayList_t> m_files = {};

  bool CheckExtension(const char* path);

public:
  bool        begin(void);
  uint32_t    GetPlayNo(void) { return m_playNo; }
  uint32_t    GetCounts(void) { return m_files.size(); }
  const char* GetPath(uint32_t playNo) { return m_files[playNo].path.c_str(); }
  void        ScanFileList(const char *dirname, uint8_t levels);
  void        SortFileList(bool shuffle = false);

  void        SetVolume(uint8_t vol);
  uint8_t     GetVolumePerCent(void);
  bool        IsPlaying(void);
  bool        FilePlay(const char* path);
  void        StopPlay(void);
  void        PauseResume(void);
  void        SetPlayNo(uint32_t playNo);
  void        PlayNext(void);
  void        PlayPrev(void);
  void        AutoPlay(void);
};
/*
void audio_info(const char *info);
void audio_id3data(const char *info);
void audio_eof_mp3(const char *info);
void audio_showstation(const char *info);
void audio_showstreamtitle(const char *info);
void audio_bitrate(const char *info);
void audio_commercial(const char *info);
void audio_icyurl(const char *info);
void audio_lasthost(const char *info);
*/
#endif // _CYD_MP3PLAYER_H_