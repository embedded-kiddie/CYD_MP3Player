/*--------------------------------------------------------------------------------
 * CYD_MP3Player class definition
 *--------------------------------------------------------------------------------*/
#include "CYD_MP3Player.h"

bool CYD_MP3Player::begin() {
  if (FS_DEV.begin(FS_CONFIG)) {
    return true;
  } else {
    Serial.println("Failed to mount the file system.");
    return false;
  }
}

/*--------------------------------------------------------------------------------
* Verify file extension. (mp3, m4a, aac, wav, flac, opus, ogg, oga)
*--------------------------------------------------------------------------------*/
bool CYD_MP3Player::VerifyExt(const char* file) {
  const char* ext[] = {".mp3", ".m4a", ".wav"};
  for (int i = 0; i < sizeof(ext) / sizeof(ext[0]); i++) {
    if (strcmp(&file[strlen(file) - strlen(ext[i])], ext[i]) == 0) {
      return true;
    }
  }
  return false;
}

/*--------------------------------------------------------------------------------
* Scan and create a list of audio m_files in a specified directory.
*--------------------------------------------------------------------------------*/
void CYD_MP3Player::ScanFileList(const char *dirname, uint8_t levels) {
  File root = m_fs.open(dirname);
  if (!root) {
    Serial.printf("Failed to open %s.\n", dirname);
    return;
  }
  if (!root.isDirectory()) {
    Serial.printf("Not a directory.\n");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    // Serial.printf("%s\n", file.path());
    bool isDir = file.isDirectory();

    // skip dot file
#ifdef SDFATFS_USED
    char name[BUF_SIZE];
    file.getName(name, sizeof(name));
    std::string path = std::string(dirname) + "/" + std::string(name);
    if (file.isHidden()) {
      // Serial.printf("%s is skipped.\n", name);
    }
#else
    const char *p = strrchr(file.path(), '/');
    if (p[p ? 1 : 0] == '.') {
      // Serial.printf("%s is skipped.\n", file.path());
    }
#endif

    else if (isDir && levels) {
#ifdef SDFATFS_USED
      ScanFileList(path.c_str(), levels - 1);
#else
      ScanFileList(file.path(), levels - 1);
#endif
    }

    else if (!isDir) {
      try {
#ifdef SDFATFS_USED
        if (VerifyExt(path.c_str())) {
          m_files.push_back({path});
        }
#else
        if (VerifyExt(file.path())) {
          m_files.push_back({file.path()});
        }
#endif
      } catch (const std::exception &e) {
        Serial.printf("Exception: %s\n", e.what());
        return;
      }
    }

    file = root.openNextFile();
  }
}

/*--------------------------------------------------------------------------------
* Sort file list
*--------------------------------------------------------------------------------*/
void CYD_MP3Player::SortFileList(bool shuffle) {
  if (shuffle) {
    std::mt19937 engine(esp_random());
    std::shuffle(m_files.begin(), m_files.end(), engine);
  } else {
    std::sort(m_files.begin(), m_files.end(), [](FileInfo_t &a, FileInfo_t &b) {
      return a.path.compare(b.path) > 0 ? true : false;
    });
  }

  for (auto& file : m_files) {
    Serial.println(file.path.c_str());
  }
}

/*--------------------------------------------------------------------------------
* Operation
*--------------------------------------------------------------------------------*/
void CYD_MP3Player::SetVolume(uint8_t vol) {
  audioSetVolume(vol);
}

uint8_t CYD_MP3Player::GetVolumePerCent(void) {
  return audioGetVolumePerCent();
}

bool CYD_MP3Player::IsPlaying(void) {
  return audioIsPlaying();
}

void CYD_MP3Player::StopPlay(void) {
  audioStopSong();
}

bool CYD_MP3Player::FilePlay(const char* path) {
  if (audioIsPlaying()) {
    audioStopSong();
  }
  if (audioConnecttoSD(path)) {
    return true;
  } else {
    Serial.printf("Cannot play %s\n", path);
    return false;
  }
}

void CYD_MP3Player::AutoPlay(void) {
  if (!audioIsPlaying() && m_files.size()) {
    if (!audioConnecttoSD(m_files[m_playNo].path.c_str())) {
      Serial.printf("skip %s\n", m_files[m_playNo].path.c_str());
      m_files.erase(m_files.begin() + m_playNo);
    } else {
      m_playNo = (m_playNo + 1) % m_files.size();
    }
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