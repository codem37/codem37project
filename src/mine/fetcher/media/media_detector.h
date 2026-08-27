// Copyright 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINE_FETCHER_MEDIA_MEDIA_DETECTOR_H_
#define MINE_FETCHER_MEDIA_MEDIA_DETECTOR_H_

#include <memory>
#include <optional>
#include <string>
#include "mine/fetcher/media/media_job.h"
#include "url/gurl.h"

namespace mine {
namespace media {

class MediaDetector {
 public:
  // Detects if a network response matches media manifest or progressive streams.
  static std::optional<MediaType> DetectMediaType(
      const GURL& url,
      const std::string& mime_type);

  // Creates a MediaJob from sniffed metadata.
  static std::unique_ptr<MediaJob> CreateJobFromSniff(
      const std::string& job_id,
      const GURL& page_url,
      const GURL& media_url,
      const std::string& mime_type);
};

}  // namespace media
}  // namespace mine

#endif  // MINE_FETCHER_MEDIA_MEDIA_DETECTOR_H_
