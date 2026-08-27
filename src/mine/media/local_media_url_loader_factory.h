// Copyright 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINE_MEDIA_LOCAL_MEDIA_URL_LOADER_FACTORY_H_
#define MINE_MEDIA_LOCAL_MEDIA_URL_LOADER_FACTORY_H_

#include <memory>
#include <string>
#include "base/memory/weak_ptr.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "mojo/public/cpp/bindings/receiver_set.h"
#include "services/network/public/cpp/self_deleting_url_loader_factory.h"
#include "services/network/public/mojom/url_loader_factory.mojom.h"

namespace mine {
namespace media {

// URL loader factory for the isolated `codem37-media://` scheme.
// Disallows arbitrary filesystem traversal; only routes through active MediaJob sessions.
class LocalMediaURLLoaderFactory : public network::SelfDeletingURLLoaderFactory {
 public:
  static constexpr char kScheme[] = "codem37-media";

  static mojo::PendingRemote<network::mojom::URLLoaderFactory> Create();

  LocalMediaURLLoaderFactory(
      mojo::PendingReceiver<network::mojom::URLLoaderFactory> receiver);
  ~LocalMediaURLLoaderFactory() override;

  // network::mojom::URLLoaderFactory:
  void CreateLoaderAndStart(
      mojo::PendingReceiver<network::mojom::URLLoader> receiver,
      int32_t request_id,
      uint32_t options,
      const network::ResourceRequest& url_request,
      mojo::PendingRemote<network::mojom::URLLoaderClient> client,
      const net::MutableNetworkTrafficAnnotationTag& traffic_annotation) override;

 private:
  base::WeakPtrFactory<LocalMediaURLLoaderFactory> weak_factory_{this};
};

}  // namespace media
}  // namespace mine

#endif  // MINE_MEDIA_LOCAL_MEDIA_URL_LOADER_FACTORY_H_
