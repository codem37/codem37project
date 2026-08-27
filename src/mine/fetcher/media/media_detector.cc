// Copyright 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mine/fetcher/media/media_detector.h"

#include "base/strings/string_util.h"

namespace mine {
namespace media {

std::optional<MediaType> MediaDetector::DetectMediaType(
    const GURL& url,
    const std::string& mime_type) {
  std::string lower_mime = base::ToLowerASCII(mime_type);
  std::string path = base::ToLowerASCII(url.path());

  // 1. HLS Detection
  if (lower_mime == "application/vnd.apple.mpegurl" ||
      lower_mime == "application/x-mpegurl" ||
      lower_mime == "audio/x-mpegurl" ||
      base::EndsWith(path, ".m3u8", base::CompareCase::INSENSITIVE_ASCII)) {
    return MediaType::kHls;
  }

  // 2. DASH Detection
  if (lower_mime == "application/dash+xml" ||
      base::EndsWith(path, ".mpd", base::CompareCase::INSENSITIVE_ASCII)) {
    return MediaType::kDash;
  }

  // 3. Progressive Media Detection
  if (base::StartsWith(lower_mime, "video/", base::CompareCase::INSENSITIVE_ASCII) ||
      base::StartsWith(lower_mime, "audio/", base::CompareCase::INSENSITIVE_ASCII) ||
      base::EndsWith(path, ".mp4", base::CompareCase::INSENSITIVE_ASCII) ||
      base::EndsWith(path, ".webm", base::CompareCase::INSENSITIVE_ASCII) ||
      base::EndsWith(path, ".mkv", base::CompareCase::INSENSITIVE_ASCII) ||
      base::EndsWith(path, ".ts", base::CompareCase::INSENSITIVE_ASCII)) {
    return MediaType::kProgressive;
  }

  return std::nullopt;
}

std::unique_ptr<MediaJob> MediaDetector::CreateJobFromSniff(
    const std::string& job_id,
    const GURL& page_url,
    const GURL& media_url,
    const std::string& mime_type) {
  auto type_opt = DetectMediaType(media_url, mime_type);
  if (!type_opt.has_value()) {
    return nullptr;
  }

  auto job = std::make_unique<MediaJob>(job_id, page_url, *type_opt);
  MediaTrack track;
  track.id = "stream_0";
  track.kind = "video";
  track.stream_url = media_url;
  job->AddTrack(track);

  return job;
}

}  // namespace media
}  // namespace mine
