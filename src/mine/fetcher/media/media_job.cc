// Copyright 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mine/fetcher/media/media_job.h"

namespace mine {
namespace media {

MediaJob::MediaJob(const std::string& job_id, const GURL& page_url, MediaType type)
    : id_(job_id), page_url_(page_url), type_(type) {}

MediaJob::~MediaJob() = default;

void MediaJob::AddTrack(const MediaTrack& track) {
  tracks_.push_back(track);
}

}  // namespace media
}  // namespace mine
