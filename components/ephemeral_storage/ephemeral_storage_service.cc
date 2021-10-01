/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/ephemeral_storage/ephemeral_storage_service.h"

#include "base/memory/ref_counted.h"
#include "base/strings/strcat.h"
#include "base/threading/sequenced_task_runner_handle.h"
#include "components/content_settings/core/browser/host_content_settings_map.h"
#include "components/content_settings/core/common/content_settings.h"
#include "components/services/storage/public/mojom/local_storage_control.mojom.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/storage_partition.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "services/network/public/mojom/cookie_manager.mojom.h"
#include "third_party/blink/public/mojom/dom_storage/storage_area.mojom.h"

namespace ephemeral_storage {

namespace {

class UrlStorageChecker : public base::RefCounted<UrlStorageChecker> {
 public:
  using Callback = base::OnceCallback<void(bool is_storage_empty)>;

  UrlStorageChecker(content::StoragePartition* storage_partition,
                    const GURL& url,
                    Callback callback)
      : storage_partition_(storage_partition),
        url_(url),
        callback_(std::move(callback)) {
    DCHECK(storage_partition_);
    DCHECK(url_.is_valid());
    DCHECK(callback_);
  }

  void StartCheck() {
    storage_partition_->GetCookieManagerForBrowserProcess()->GetCookieList(
        url_, net::CookieOptions::MakeAllInclusive(),
        base::BindOnce(&UrlStorageChecker::OnGetCookieList, this));
  }

 private:
  friend class base::RefCounted<UrlStorageChecker>;
  ~UrlStorageChecker() = default;

  void OnGetCookieList(
      const std::vector<net::CookieWithAccessResult>& included_cookies,
      const std::vector<net::CookieWithAccessResult>& excluded_cookies) {
    LOG(ERROR) << "included_cookies " << included_cookies.size()
               << " excluded_cookies " << excluded_cookies.size();
    if (!included_cookies.empty()) {
      std::move(callback_).Run(false);
      return;
    }

    storage_partition_->GetLocalStorageControl()->BindStorageArea(
        blink::StorageKey(url::Origin::Create(url_)),
        local_storage_area_.BindNewPipeAndPassReceiver());
    local_storage_area_->GetAll(
        {}, base::BindOnce(&UrlStorageChecker::OnGetLocalStorageData, this));
  }

  void OnGetLocalStorageData(
      std::vector<blink::mojom::KeyValuePtr> local_storage_data) {
    if (!local_storage_data.empty()) {
      std::move(callback_).Run(false);
      return;
    }

    std::move(callback_).Run(true);
  }

  content::StoragePartition* storage_partition_ = nullptr;
  GURL url_;
  mojo::Remote<blink::mojom::StorageArea> local_storage_area_;
  Callback callback_;
};

}  // namespace

EphemeralStorageService::EphemeralStorageService(
    content::BrowserContext* context,
    HostContentSettingsMap* host_content_settings_map)
    : context_(context), host_content_settings_map_(host_content_settings_map) {
  DCHECK(context_);
  DCHECK(host_content_settings_map_);
}

EphemeralStorageService::~EphemeralStorageService() {}

void EphemeralStorageService::CanEnable1PESForUrl(
    const GURL& url,
    base::OnceCallback<void(bool can_enable_1pes)> callback) const {
  base::SequencedTaskRunnerHandle::Get()->PostTask(
      FROM_HERE,
      base::BindOnce(&UrlStorageChecker::StartCheck,
                     base::MakeRefCounted<UrlStorageChecker>(
                         context_->GetStoragePartitionForUrl(url, true), url,
                         std::move(callback))));
}

void EphemeralStorageService::Enable1PESForUrl(const GURL& url, bool enable) {
  host_content_settings_map_->SetContentSettingCustomScope(
      ContentSettingsPattern::FromString(
          base::StrCat({"[*.]", url.host_piece(), ":*"})),
      ContentSettingsPattern::Wildcard(), ContentSettingsType::COOKIES,
      enable ? CONTENT_SETTING_SESSION_ONLY : CONTENT_SETTING_DEFAULT);
}

bool EphemeralStorageService::Is1PESEnabledForUrl(const GURL& url) const {
  return host_content_settings_map_->GetContentSetting(
             url, url, ContentSettingsType::COOKIES) ==
         CONTENT_SETTING_SESSION_ONLY;
}

}  // namespace ephemeral_storage