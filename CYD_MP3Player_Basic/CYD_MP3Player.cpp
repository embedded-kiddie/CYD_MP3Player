/*--------------------------------------------------------------------------------
 * CYD_MP3Player class definition
 *--------------------------------------------------------------------------------*/
#include "CYD_MP3Player.h"

/*--------------------------------------------------------------------------------
 * Begin with SD or SdFat
 *--------------------------------------------------------------------------------*/
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
bool CYD_MP3Player::CheckExtension(const char* path) {
  const char* ext[] = {".mp3", ".m4a", ".wav"};
  for (int i = 0; i < sizeof(ext) / sizeof(ext[0]); i++) {
    if (strcmp(&path[strlen(path) - strlen(ext[i])], ext[i]) == 0) {
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
    std::string path = std::string(dirname);
    path += (path.at(path.size() - 1) == '/' ? "" : "/") + std::string(name);
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
        if (CheckExtension(path.c_str())) {
          m_files.push_back({path});
        }
#else
        if (CheckExtension(file.path())) {
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
    std::sort(m_files.begin(), m_files.end(), [](PlayList_t &a, PlayList_t &b) {
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

bool CYD_MP3Player::IsLastSong(void) {
  return m_playNo == m_files.size() - 1;
}

bool CYD_MP3Player::FilePlay(const char* path) {
  audioStopSong();
  if (audioConnecttoSD(path)) {
    return true;
  } else {
    Serial.printf("Cannot play %s\n", path);
    return false;
  }
}

void CYD_MP3Player::StopPlay(void) {
  audioStopSong();
}

void CYD_MP3Player::PauseResume(void) {
  audioPauseResume();
}

void CYD_MP3Player::SetPlayNo(uint32_t playNo, bool stop) {
  if (stop) {
    audioStopSong();
  }
  uint32_t size = m_files.size();
  if (size) {
    m_playNo = (playNo + size) % size;
  }
}

void CYD_MP3Player::PlayNext(bool stop) {
  SetPlayNo(m_playNo + 1, stop);
}

void CYD_MP3Player::PlayPrev(bool stop) {
  SetPlayNo(m_playNo - 1, stop);
}

void CYD_MP3Player::AutoPlay(void) {
  if (!audioIsPlaying() && m_files.size()) {
    if (!audioConnecttoSD(m_files[m_playNo].path.c_str())) {
      Serial.printf("skip %s\n", m_files[m_playNo].path.c_str());
    }
  }
}