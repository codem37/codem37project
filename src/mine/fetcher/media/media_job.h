// Copyright 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINE_FETCHER_MEDIA_MEDIA_JOB_H_
#define MINE_FETCHER_MEDIA_MEDIA_JOB_H_

#include <cstdint>
#include <string>
#include <vector>
#include "url/gurl.h"

namespace mine {
namespace media {

enum class MediaType {
  kProgressive, // MP4, WebM
  kHls,         // M3U8
  kDash,        // MPD
};

enum class MediaJobState {
  kDetected,
  kDownloading,
  kCompleted,
  kFailed,
  kCancelled,
};

struct MediaTrack {
  std::string id;
  std::string kind; // "video", "audio"
  std::string resolution; // e.g. "1080p (1920x1080)"
  uint64_t bandwidth = 0;
  std::string codecs;
  GURL stream_url;
};

class MediaJob {
 public:
  MediaJob(const std::string& job_id, const GURL& page_url, MediaType type);
  ~MediaJob();

  const std::string& id() const { return id_; }
  const GURL& page_url() const { return page_url_; }
  MediaType type() const { return type_; }
  MediaJobState state() const { return state_; }

  void AddTrack(const MediaTrack& track);
  const std::vector<MediaTrack>& tracks() const { return tracks_; }

  void SetState(MediaJobState state) { state_ = state; }
  void SetTitle(const std::string& title) { title_ = title; }
  const std::string& title() const { return title_; }

 private:
  std::string id_;
  GURL page_url_;
  std::string title_;
  MediaType type_;
  MediaJobState state_ = MediaJobState::kDetected;
  std::vector<MediaTrack> tracks_;
};

}  // namespace media
}  // namespace mine

#endif  // MINE_FETCHER_MEDIA_MEDIA_JOB_H_
