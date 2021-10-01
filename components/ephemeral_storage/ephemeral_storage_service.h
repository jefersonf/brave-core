/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_EPHEMERAL_STORAGE_EPHEMERAL_STORAGE_SERVICE_H_
#define BRAVE_COMPONENTS_EPHEMERAL_STORAGE_EPHEMERAL_STORAGE_SERVICE_H_

#include "base/callback.h"
#include "base/memory/scoped_refptr.h"
#include "base/memory/weak_ptr.h"
#include "base/observer_list.h"
#include "base/scoped_observation.h"
#include "base/values.h"
#include "components/keyed_service/core/keyed_service.h"
#include "url/gurl.h"

class HostContentSettingsMap;

namespace content {
class BrowserContext;
class StoragePartition;
}  // namespace content

namespace ephemeral_storage {

class EphemeralStorageService
    : public KeyedService,
      public base::SupportsWeakPtr<EphemeralStorageService> {
 public:
  EphemeralStorageService(content::BrowserContext* context,
                          HostContentSettingsMap* host_content_settings_map);
  ~EphemeralStorageService() override;

  void CanEnable1PESForUrl(
      const GURL& url,
      base::OnceCallback<void(bool can_enable_1pes)> callback) const;
  void Enable1PESForUrl(const GURL& url, bool enable);
  bool Is1PESEnabledForUrl(const GURL& url) const;

 private:
  content::BrowserContext* context_ = nullptr;
  HostContentSettingsMap* host_content_settings_map_ = nullptr;
};

}  // namespace ephemeral_storage

#endif  // BRAVE_COMPONENTS_EPHEMERAL_STORAGE_EPHEMERAL_STORAGE_SERVICE_H_
