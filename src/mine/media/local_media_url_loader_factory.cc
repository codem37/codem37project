// Copyright 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mine/media/local_media_url_loader_factory.h"

#include <utility>
#include "base/logging.h"
#include "net/base/net_errors.h"
#include "services/network/public/mojom/url_response_head.mojom.h"

namespace mine {
namespace media {

LocalMediaURLLoaderFactory::LocalMediaURLLoaderFactory(
    mojo::PendingReceiver<network::mojom::URLLoaderFactory> receiver)
    : network::SelfDeletingURLLoaderFactory(std::move(receiver)) {}

LocalMediaURLLoaderFactory::~LocalMediaURLLoaderFactory() = default;

mojo::PendingRemote<network::mojom::URLLoaderFactory>
LocalMediaURLLoaderFactory::Create() {
  mojo::PendingRemote<network::mojom::URLLoaderFactory> pending_remote;
  new LocalMediaURLLoaderFactory(pending_remote.InitWithNewPipeAndPassReceiver());
  return pending_remote;
}

void LocalMediaURLLoaderFactory::CreateLoaderAndStart(
    mojo::PendingReceiver<network::mojom::URLLoader> receiver,
    int32_t request_id,
    uint32_t options,
    const network::ResourceRequest& url_request,
    mojo::PendingRemote<network::mojom::URLLoaderClient> client,
    const net::MutableNetworkTrafficAnnotationTag& traffic_annotation) {
  mojo::Remote<network::mojom::URLLoaderClient> client_remote(std::move(client));

  // Security Invariant: Verify scheme is strictly "codem37-media"
  if (!url_request.url.SchemeIs(kScheme)) {
    client_remote->OnComplete(network::URLLoaderCompletionStatus(net::ERR_DISALLOWED_URL_SCHEME));
    return;
  }

  // Security Invariant: Disallow filesystem paths (C:\, /, ..)
  std::string media_id = url_request.url.host();
  if (media_id.empty() || media_id.find('/') != std::string::npos ||
      media_id.find('\\') != std::string::npos ||
      media_id.find("..") != std::string::npos) {
    LOG(ERROR) << "[codem37::Media] Blocked invalid/traversal media ID: " << media_id;
    client_remote->OnComplete(network::URLLoaderCompletionStatus(net::ERR_ACCESS_DENIED));
    return;
  }

  auto response_head = network::mojom::URLResponseHead::New();
  response_head->mime_type = "video/mp4";

  // Feed stream headers
  client_remote->OnReceiveResponse(std::move(response_head), mojo::ScopedDataPipeConsumerHandle(),
                                   std::nullopt);
  client_remote->OnComplete(network::URLLoaderCompletionStatus(net::OK));
}

}  // namespace media
}  // namespace mine
