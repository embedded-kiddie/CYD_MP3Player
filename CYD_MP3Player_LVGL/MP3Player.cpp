//================================================================================
// MP3Player class definition
//================================================================================
#include "CYD28_audio.h"
#include "MP3Player.h"

#include <ctype.h>    // isdigit(), isprint()
#include <stdlib.h>   // atoi()
#include <string.h>   // strncpy(), strtok_r(), strrchr()
#include <string>     // std::string
#include <random>     // std::mt19937
#include <algorithm>  // std::shuffle, std::sort
#include <functional> // std::hash

//--------------------------------------------------------------------------------
// Begin with SD or SdFat
//--------------------------------------------------------------------------------
bool MP3Player::begin(const char *root, uint8_t volume) {
  // Set root path
  m_base = root;
  if (m_base.back() != '/') {
    m_base.append("/");
  }

  m_root = m_base; // mainly m_root is used

  // Initialize SD card
  if (!SD.begin(SD_CONFIG)) {
    m_error = "failed to mount: " + m_root;
    DBG_EXEC(printf("%s\n", m_error.c_str()));
    return false;
  }

#if MY_USE_FS_ARDUINO_SD
  // LVGL SD File System for displaying cover photos
  lv_fs_arduino_sd_init();
#endif

  SetVolume(volume);
  return true;
}

//--------------------------------------------------------------------------------
// Scan the SD card and create an album list (i.e. a node tree)
//--------------------------------------------------------------------------------
uint32_t MP3Player::ScanAlbumDirs(void) {
  uint32_t time;
  DBG_EXEC({
    printf("%s: Free heap %7lu bytesm / Minimum heap %7lu bytes\n", __func__,
      heap_caps_get_free_size(MALLOC_CAP_DEFAULT),
      heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT));
    time = lv_tick_get();
  });

  if (m_tree == NULL) {
    File dir = SD.open(m_root.c_str());
    if (!dir) {
      m_error = "Can't open " + m_root;
      return 0;
    }

    m_tree = new Node(m_root.c_str());
    DBG_ASSERT(m_tree);
    m_tree->scan_dir(dir);
    dir.close();
  }

  DBG_EXEC({
    //m_tree->dump_tree();
    printf("%s: %lu [msec]\n", __func__, lv_tick_elaps(time));
  });

  return m_tree->get_n_leafs();
}

//--------------------------------------------------------------------------------
// Randomly scan a specified number of audio files
//--------------------------------------------------------------------------------
uint32_t MP3Player::ScanAudioFiles(uint8_t partition, bool shuffle) {
  DBG_ASSERT(m_tree && m_list.size() == 0);
  uint32_t time;
  DBG_EXEC({
    printf("%s: Free heap %7lu bytesm / Minimum heap %7lu bytes\n", __func__,
      heap_caps_get_free_size(MALLOC_CAP_DEFAULT),
      heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT));
    time = lv_tick_get();
  });

  std::mt19937 engine(esp_random());

  const size_t n_leafs = m_tree->get_n_leafs(); // Number of albums
  const size_t n_audio = m_tree->get_n_audio(); // Number of audio files
  #define MIN(a, b) ((a) < (b) ? (a) : (b))
  uint32_t max_files = MIN(MP3_PERTITION_FILES, n_audio);

  if (partition) {
    for (int i = 0, key = 0; max_files > i && key < n_leafs; key++) {
      i = scan_files(key);
    }
  }

  else {
    std::vector<uint16_t> pot;
    for (int i = 0; i < n_leafs; i++) {
      pot.push_back(i);
    }

    // Prevent multiple selections of the same album
    for (int i = 0, n_leafs = pot.size(); max_files > i && n_leafs; n_leafs = pot.size()) {
      uint32_t key = engine() % n_leafs;
      i = scan_files(pot[key]);
      pot.erase(pot.begin() + key); // Delete the key-th element
    }
  }

  if (m_list.size() == 0) {
    m_error = "No music to play";
  }

  else if (shuffle) {
    std::shuffle(m_list.begin(), m_list.end(), engine);
  }

  DBG_EXEC({
    //dump_files();
    printf("%s: %lu [msec]\n", __func__, lv_tick_elaps(time));
    printf("%s: Free heap %7lu bytesm / Minimum heap %7lu bytes\n", __func__,
      heap_caps_get_free_size(MALLOC_CAP_DEFAULT),
      heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT));
    printf("%s: %d files\n", __func__, m_list.size());
  });

  return m_list.size();
}

//--------------------------------------------------------------------------------
// Get the play list for a specified track
//--------------------------------------------------------------------------------
MP3List_t* MP3Player::GetPlayList(uint32_t playNo) {
  if (m_list.size()) {
    return & m_list[playNo];
  } else {
    return NULL;
  }
}

//--------------------------------------------------------------------------------
// Get path to the audio file
//--------------------------------------------------------------------------------
std::string MP3Player::GetDirPath(uint32_t playNo) {
  MP3List_t *list = GetPlayList(playNo);
  if (list) {
    return m_tree->find_path(list->key) + "/";
  } else {
    return "";
  }
}

std::string MP3Player::GetFilePath(uint32_t playNo) {
  MP3List_t *list = GetPlayList(playNo);
  if (list) {
    return m_tree->find_path(list->key) + "/" + list->name;
  } else {
    return "";
  }
}

bool MP3Player::SaveMetaData(uint32_t playNo, MP3Meta_t *meta) {
  if (audioIsPlaying()) {
    return false;
  }

  MP3List_t *list = GetPlayList(playNo); // Never NULL
  std::string path = m_tree->find_path(list->key);
  std::string file = path + "/" ALBUM_META_FILE;

  File fd = SD.open(file.c_str(), FILE_READ);
  if (fd) {
    const size_t size = fd.SDFS_SIZE();
    const size_t n = size / sizeof(MP3Hash_t);
    MP3Hash_t *meta_hash = new MP3Hash_t[n];
    DBG_ASSERT(meta_hash);

    if (!meta_hash) {
      fd.close();
      return false;
    }

    fd.read((SDFS_VOID*)meta_hash, size);
    fd.close();

    // Search for meta data with matching hash
    std::hash<std::string> MakeHash;
    size_t hash = MakeHash(list->name);
    for (int i = 0; i < n; i++) {
      if (meta_hash[i].hash == hash) {
        meta->saved = meta->selected; // Mark 'selected' as saved
        meta_hash[i].meta = *meta;
        break;
      }
    }

    fd = SD.open(file.c_str(), FILE_WRITE);
    if (fd) {
      fd.seek(0);
      fd.write((SDFS_VOID*)meta_hash, size); // should check return value!
      fd.close();
    }

    DBG_EXEC(printf("%s: %s\n", __func__, list->name.c_str()));

    delete[] meta_hash;
    return true;
  }

  return false;
}

//--------------------------------------------------------------------------------
// Get metadata from play list and save it to a dedicated file
//--------------------------------------------------------------------------------
void MP3Player::GetMetaData(uint32_t playNo, MP3Meta_t *meta) {
  MP3List_t *list = GetPlayList(playNo);
  if (list) {
    *meta = list->meta;
  } else {
    *meta = {}; // Never get here
  }
}

bool MP3Player::PutMetaData(uint32_t playNo, MP3Meta_t *meta) {
  MP3List_t *list = GetPlayList(playNo);
  if (list) {
    list->meta = *meta;
    return SaveMetaData(playNo, meta);
  }
  return false; // Never get here
}

bool MP3Player::UpdateMetaData(void) {
  bool ret = true;
  int i = 0;
  for (auto& list : m_list) {
    if (list.meta.saved != list.meta.selected) {
      ret &= SaveMetaData(i, &list.meta);
    }
    ++i;
  }
  return ret;
}

//--------------------------------------------------------------------------------
// Clear all the nodes in tree
//--------------------------------------------------------------------------------
void MP3Player::ClearAudioFiles(void) {
  m_list.clear();
}

//--------------------------------------------------------------------------------
// Load the photo number stored in the metadata on the SD card
//--------------------------------------------------------------------------------
uint32_t MP3Player::GetPhotoNo(uint32_t playNo) {
  uint32_t pictNo = 0;

  // Gets the photo number recorded in ALBUM_PHOTO_FILE.txt
  std::string path = GetDirPath(playNo);
  path.append(ALBUM_PHOTO_FILE ".txt");

  File fd = SD.open(path.c_str(), FILE_READ);
  if (fd) {
#ifdef  SDFATFS_USED
    String buf = "";
    while (fd.available()) {
      buf += fd.readString();
    }
    fd.close();
    if (isdigit(buf[0])) {
      pictNo = atoi(buf.c_str());
    }
#else // SD
    char buf[BUF_SIZE];
    fd.read((uint8_t*)buf, sizeof(buf));
    fd.close();
    buf[sizeof(buf) - 1] = '\0';
    if (isdigit(buf[0])) {
      pictNo = atoi(buf);
    }
#endif // SdFat or SD
  }

  return pictNo;
}

//--------------------------------------------------------------------------------
// Get ID3 tags (title, album, artist) from the play list
//--------------------------------------------------------------------------------
void MP3Player::GetID3Tags(uint32_t playNo, MP3Tags_t &tags) {
  MP3List_t *list = GetPlayList(playNo);
  if (list) {
    tags.meta = list->meta;

    int n = 0;
    char *ptr, *token, *tmp[8], copy[BUF_SIZE];
    std::string path = GetFilePath(playNo);

    // UTF-8 multibyte characters should be handled correctly
    strncpy(copy, path.c_str(), sizeof(copy));
    copy[sizeof(copy) - 1] = '\0';

    token = strtok_r(copy, "/", &ptr);
    while (token != NULL && n < 8) {
      tmp[n++] = token;
      token = strtok_r(NULL, "/", &ptr);
    }

    if (--n >= 0) {
      ptr = strrchr(tmp[n], '.'); // ".mp3", ".m4a", ".wav"
      if (ptr) {
        *ptr = '\0';
      }
      if (isdigit(*tmp[n])) { // "1-01 title"
        ptr = strchr(tmp[n], ' ');
        ptr = ptr ? ptr + 1 : tmp[n];
        tags.title = ptr;
      } else {
        tags.title = tmp[n];
      }
      if (--n >= 0) {
        tags.album = tmp[n];
        if (--n >= 0) {
          tags.artist = tmp[n];
        }
      }
    }
  } else {
    tags = {}; // Never get here
  }
}

//--------------------------------------------------------------------------------
// Set / Get error message
//--------------------------------------------------------------------------------
void MP3Player::SetError(const char* msg) {
  m_error = msg;
}

const char* MP3Player::GetError(void) {
  return m_error.c_str();
}

//--------------------------------------------------------------------------------
// Operation
//--------------------------------------------------------------------------------
void MP3Player::SetVolume(uint8_t vol) {
  audioSetVolume(vol);
}

uint8_t MP3Player::GetVolumePerCent(void) {
  return audioGetVolumePerCent();
}

bool MP3Player::IsPlaying(void) {
  return audioIsPlaying();
}

bool MP3Player::IsLastSong(bool selected) {
  const int n = m_list.size();
  if (selected) {
    for (int i = m_playNo + 1; i < n; i++) {
      MP3List_t *list = GetPlayList(i);
      if (list->meta.selected) {
        return false;
      }
    }
    return true;
  } else {
    return m_playNo == n - 1;
  }
}

bool MP3Player::FilePlay(const char* path) {
  audioStopSong();
  if (audioConnecttoSD(path)) {
    return true;
  } else {
    m_error = "Cannot play " + std::string(path);
    return false;
  }
}

void MP3Player::StopPlay(void) {
  audioStopSong();
}

void MP3Player::PauseResume(void) {
  audioPauseResume();
}

void MP3Player::SetPlayNo(uint32_t playNo, bool stop) {
  if (stop) {
    audioStopSong();
  }
  uint32_t size = m_list.size();
  if (size) {
    m_playNo = (playNo + size) % size;
  }
}

void MP3Player::PlayNext(bool stop) {
  SetPlayNo(m_playNo + 1, stop);
}

void MP3Player::PlayPrev(bool stop) {
  SetPlayNo(m_playNo - 1, stop);
}

bool MP3Player::IsSelected(void) {
  if (m_list.size()) {
    return m_list[m_playNo].meta.selected;
  } else {
    return false;
  }
}

//--------------------------------------------------------------------------------
// Check if the next song is selected
//--------------------------------------------------------------------------------
bool MP3Player::NextSelected(bool next, bool loop, bool stop) {
  const int N = m_list.size();
  const int m = (m_playNo + (next ? 1 : -1) + N) % N;
  const int n = N - (loop ? 0 : m);

  for (int i = 0; i < n; i++) {
    int j = (m + (next ? i : -i) + N) % N;
    if (m_list[j].meta.selected) {
      SetPlayNo(j, stop);
      return true;
    }
  }

  return false;
}

bool MP3Player::AutoPlay(void) {
  if (!audioIsPlaying()) {
    std::string path = GetFilePath(m_playNo);
    if (!audioConnecttoSD(path.c_str())) {
      m_error = "Failed to play " + path;
      DBG_EXEC(printf("%s\n", m_error.c_str()));
      return false;
    }
  }
  return true;
}

//--------------------------------------------------------------------------------
// Optional functions for audio-I2S (defined in CYD_Audio.h as a weak function)
//--------------------------------------------------------------------------------
#if   false
void audio_info(const char *info) {
  Serial.print("info        ");
  Serial.println(info);
}
void audio_id3data(const char *info) {  // id3 metadata
  Serial.print("id3tags     ");
  Serial.println(info);
}
void audio_eof_mp3(const char *info) {  // end of file
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
void audio_commercial(const char *info) {  // duration in sec
  Serial.print("commercial  ");
  Serial.println(info);
}
void audio_icyurl(const char *info) {  // homepage
  Serial.print("icyurl      ");
  Serial.println(info);
}
void audio_lasthost(const char *info) {  // stream URL played
  Serial.print("lasthost    ");
  Serial.println(info);
}
#endif