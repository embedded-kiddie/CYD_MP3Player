//================================================================================
// MP3Player class definition
//  Auther: embedded-kiddie (https://github.com/embedded-kiddie)
//  Released under the GPLv3 (https://www.gnu.org/licenses/gpl-3.0.html)
//================================================================================
#ifndef _MP3PLAYER_H_
#define _MP3PLAYER_H_

#include "config.h"
#include "tree.hpp"
#include <string>     // std::string
#include <vector>     // std::vector
#include <functional> // std::hash

//--------------------------------------------------------------------------------
// Possible values for `SetVolume()`
//--------------------------------------------------------------------------------
#define MP3_VOLUME_MIN  0
#define MP3_VOLUME_INI  6
#define MP3_VOLUME_MAX  21

//--------------------------------------------------------------------------------
// Estimated number of albums/audio files and memory consumption
// Note: Bluetooth requires at least an additional 56K bytes of RAM
//--------------------------------------------------------------------------------
#define MP3_PERTITION_FILES   625           // Number of audio files per partition
#define MP3_PERTITION_ALBUMS   50           // Number of albums per partition
#define MP3_HEAP_SIZE_ALBUM   150           // Averary memory consumption per album
#define MP3_HEAP_SIZE_FILE    100           // Averary memory consumption per audio file
#define MP3_HEAP_MAX_FREE     (150 * 1024U) // Estimated maximum free heap memory size
#define MP3_HEAP_MIN_FREE     ( 94 * 1024U) // (MP3_PERTITION_ALBUMS * MP3_HEAP_SIZE_ALBUM + MP3_PERTITION_FILES * MP3_HEAP_SIZE_FILE) * FoS

//--------------------------------------------------------------------------------
// Meta data for MP3 audio file
//--------------------------------------------------------------------------------
typedef struct {
  uint16_t saved    : 1;  // Saved selected flag
  uint16_t selected : 1;  // Selected or not
  uint16_t duration : 14; // [sec]
} MP3Meta_t;

// Metadata paired with a hash of the file name
typedef struct {
  size_t    hash;   // A hash representing the file name
  MP3Meta_t meta;   // A data set that may change dynamically
} MP3Hash_t;

// https://en.wikipedia.org/wiki/ID3
typedef struct {
  MP3Meta_t meta;
  std::string title;
  std::string artist;
  std::string album;
} MP3Tags_t;

//--------------------------------------------------------------------------------
// Play list for MP3 audio file
//--------------------------------------------------------------------------------
typedef struct {
  uint16_t key;     // The key of the node in the tree
  MP3Meta_t meta;   // Meta data for audio file
  std::string name; // Audio file name
} MP3List_t;

typedef std::vector<MP3List_t> PlayList_t;

class MP3Player {
public:
  MP3Player() {}
  ~MP3Player() {
    m_list.clear();
    if (m_tree) {
      delete m_tree;
      m_tree = NULL;
    }
  }

private:
  std::string m_base = "/";
  std::string m_root = "/";
  std::string m_error = "";

  MP3List_t*  GetPlayList (uint32_t playNo);
  bool        SaveMetaData(uint32_t playNo, MP3Meta_t *meta);

public:
  Node *      m_tree = NULL;
  uint32_t    m_playNo = 0;
  PlayList_t  m_list = {};

  bool        begin(const char *root, uint8_t vol = MP3_VOLUME_INI);
  uint32_t    ScanAlbumDirs(void);
  uint32_t    ScanAudioFiles(uint8_t partition, bool shuffle = true);
  void        SetSubDir(const char* name) { m_root = m_base + name; }
  const char* GetSubDir(void) { return m_root.c_str(); }
  uint32_t    GetPlayNo(void) { return m_playNo; }
  uint32_t    GetCounts(void) { return m_list.size(); }
  std::string GetDirPath  (uint32_t playNo);
  std::string GetFilePath (uint32_t playNo);
  uint32_t    GetPhotoNo  (uint32_t playNo);
  void        GetID3Tags  (uint32_t playNo, MP3Tags_t &tags);
  void        GetMetaData (uint32_t playNo, MP3Meta_t *meta);
  bool        PutMetaData (uint32_t playNo, MP3Meta_t *meta);
  bool        UpdateMetaData(void);
  void        DeleteNodeTree(void) { if (m_tree) { delete m_tree; m_tree = NULL; } }
  void        ClearAudioFiles(void);
  void        SetError(const char* msg);
  const char* GetError(void);

  void        SetVolume(uint8_t vol);
  uint8_t     GetVolumePerCent(void);
  bool        IsPlaying(void);
  bool        IsLastSong(bool selected);
  bool        FilePlay(const char *path);
  void        StopPlay(void);
  void        PauseResume(void);
  void        SetPlayNo(uint32_t playNo, bool stop = true);
  void        PlayNext(bool stop = true);
  void        PlayPrev(bool stop = true);
  bool        IsSelected(void);
  bool        NextSelected(bool next, bool loop, bool stop = true);
  bool        AutoPlay(void);

private:
  //--------------------------------------------------------------------------------
  // Add audio file at the end of the play list
  //--------------------------------------------------------------------------------
  void append(const char * name, uint16_t key) {
    try {
      m_list.push_back({
        .key  = key,
        .meta = {},
        .name = name
      });
    } catch (const std::exception &e) {
      assert(false); //  e.what()
    }
  }

  //--------------------------------------------------------------------------------
  // Verify file extension. (mp3, m4a, aac, wav, flac, opus, ogg, oga)
  //--------------------------------------------------------------------------------
  bool check_ext(const char *path) {
    if (MP3_IS_VALID(path)) {
      const char* const ext[] = MP3_FILE_EXT;
      for (int i = 0; i < sizeof(ext) / sizeof(ext[0]); i++) {
        if (strcmp(&path[strlen(path) - strlen(ext[i])], ext[i]) == 0) {
          return true;
        }
      }
    }
    return false;
  }

  bool check_mp3(File &fd, std::string &name) {
#ifdef USE_SDFAT
    char buf[BUF_SIZE];
    fd.getName(buf, sizeof(buf));
    if (check_ext(buf)) {
      name = buf;
      return true;
    }
#else
    if (check_ext(fd.name())) {
      name = fd.name();
      return true;
    }
#endif
    return false;
  }

  void dump_files(void) {
    int i = 0; 
    for (auto &f : m_list) {
      std::string path = m_tree->find_path(f.key);
      printf("No %3d: %d/%d, %3d, %s/%s (%d/%d)\n", i++,
            f.meta.saved, f.meta.selected, f.meta.duration, path.c_str(),
            f.name.c_str(), f.name.size(), f.name.capacity());
    }
    printf("Total: %d\n", m_list.size());
  }

  //--------------------------------------------------------------------------------
  // Scan audio files and make a play list
  //--------------------------------------------------------------------------------
  uint32_t scan_files(uint32_t key) {
    std::hash<std::string> MakeHash;

    Node *node = m_tree->find_node(key);
    DBG_ASSERT(node);

    if (node->meta.checked == LEAF_SELECTED) {
      uint32_t k = m_list.size();
      const char *path = m_tree->get_path();
      File fd, dir = SD.open(path);
      while (fd = dir.openNextFile()) {
        std::string name;
        if (check_mp3(fd, name)) {
          append(name.c_str(), key);
        }
        fd.close();
      }
      dir.close();

      // Sort the list in order to arrange metadata in order
      std::sort(m_list.begin() + k, m_list.end(), [](MP3List_t &a, MP3List_t &b) {
        return a.name.compare(b.name) < 0 ? true : false; // Ascending order
      });

      // Check and fix album metadata integrity
      const int n = m_list.size() - k;
      MP3Hash_t *meta_src = new MP3Hash_t[n];
      DBG_ASSERT(meta_src); // Out of memory

      if (meta_src) {
        size_t src = sizeof(MP3Hash_t) * n;
        memset((void*)meta_src, 0, src);
        for (int i = 0; i < n; i++) {
          meta_src[i].hash = MakeHash(m_list[k + i].name);
        }

        // Read an existing meta data file
        int counts = 0;
        size_t dst = 0;
        std::string file = path;
        file += "/" ALBUM_META_FILE;
        fd = SD.open(file.c_str(), FILE_READ);

        if (fd) {
          dst = fd.SDFS_SIZE();
          const int m = dst / sizeof(MP3Hash_t);
          MP3Hash_t *meta_dst = new MP3Hash_t[m];
          DBG_ASSERT(meta_dst); // Out of memory

          if (meta_dst) {
            dst = fd.read((SDFS_VOID*)meta_dst, dst);

            // Find a matching hash and update meta data
            for (int i = 0; i < n; i++) {
              for (int j = 0; j < m; j++) {
                if (meta_src[i].hash == meta_dst[j].hash) {
                  m_list[k + i].meta = meta_src[i].meta = meta_dst[j].meta;
                  ++counts;
                  break;
                }
              }
            }

            delete[] meta_dst;
          }

          fd.close();
        }

        // Update if mismatched
        if (src != dst || n != counts) {
          if (fd = SD.open(file.c_str(), FILE_WRITE)) {
            fd.seek(0);
            fd.write((SDFS_VOID*)meta_src, src);
            fd.close();
          }
        }

        delete[] meta_src;
      }
    }

    return m_list.size();
  }
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
#endif // _MP3PLAYER_H_