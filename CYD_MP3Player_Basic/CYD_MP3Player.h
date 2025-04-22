/*--------------------------------------------------------------------------------
 * CYD_MP3Player class definition
 *--------------------------------------------------------------------------------*/
#ifndef _CYD_MP3PLAYER_
#define _CYD_MP3PLAYER_

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
  std::string album;
  std::string artist;
  uint32_t    duration;
} FileInfo_t;

/*--------------------------------------------------------------------------------
 * Definition of SPI file system for audio files
 * NOTE: uncomment the followings to use SdFat 
 *  "#define SDFATFS_USED" in CYD_Audio.h
 *  "#define USE_UTF8_LONG_NAMES 1" in SdFatConfig.h
 *--------------------------------------------------------------------------------*/
#define SD_CLOCK  20000000  // 1MHz --> 20MHz and up
#define SD_CS     SS

#if   defined (SDFATFS_USED)
#define FS_DEV    SD_SDFAT
#define FS_CONFIG SD_CS, SD_CLOCK
#define BUF_SIZE  64
#elif defined (_SD_H_)
#define FS_DEV    SD
#define FS_CONFIG SD_CS, SPI, SD_CLOCK
#endif

class CYD_MP3Player {
private:
  int m_playNo = 0;
  fs::FS & m_fs = FS_DEV;
  std::vector<FileInfo_t> m_files = {};

  bool VerifyExt(const char* file);

public:
  bool    begin(void);
  void    ScanFileList(const char *dirname, uint8_t levels);
  void    SortFileList(bool shuffle = false);
  void    SetVolume(uint8_t vol);
  uint8_t GetVolumePerCent(void);
  bool    IsPlaying(void);
  void    StopPlay(void);
  bool    FilePlay(const char* path);
  void    AutoPlay(void);
};

void audio_info(const char *info);
void audio_id3data(const char *info);
void audio_eof_mp3(const char *info);
void audio_showstation(const char *info);
void audio_showstreamtitle(const char *info);
void audio_bitrate(const char *info);
void audio_commercial(const char *info);
void audio_icyurl(const char *info);
void audio_lasthost(const char *info);

#endif // _CYD_MP3PLAYER_