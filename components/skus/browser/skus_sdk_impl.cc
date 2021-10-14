// Copyright (c) 2021 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#include "brave/components/skus/browser/skus_sdk_impl.h"

#include <memory>
#include <utility>
#include <vector>

#include "base/threading/sequenced_task_runner_handle.h"
#include "brave/components/skus/browser/br-rs/brave-rewards-cxx/src/wrapper.hpp"
#include "brave/components/skus/browser/pref_names.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "components/prefs/pref_service.h"
#include "net/base/load_flags.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "services/preferences/public/cpp/dictionary_value_update.h"
#include "services/preferences/public/cpp/scoped_pref_update.h"
#include "url/gurl.h"

namespace {

// TODO(bsclifton): fix me. I set a completely arbitrary size!
const int kMaxResponseSize = 1000000;  // 1Mb

brave_rewards::SkusSdkImpl* g_SkusSdk = NULL;

// START: hack code - remove me
// rust::String's std::string operator gave linker errors :(
// .c_str() not usable because function itself isn't const
// .data is not terminating string
std::string ruststring_2_stdstring(rust::String in) {
  std::string out = "";
  rust::String::iterator it = in.begin();
  while (it != in.end()) {
    out += static_cast<char>(*it);
    it++;
  }
  return out;
}
std::string ruststr_2_stdstring(rust::cxxbridge1::Str in) {
  std::string out = "";
  rust::cxxbridge1::Str::iterator it = in.begin();
  while (it != in.end()) {
    out += static_cast<char>(*it);
    it++;
  }
  return out;
}
// END: hack code - remove me

class SkusSdkFetcher {
 public:
  explicit SkusSdkFetcher(scoped_refptr<network::SharedURLLoaderFactory>);
  ~SkusSdkFetcher();

  void BeginFetch(
      const brave_rewards::HttpRequest& req,
      rust::cxxbridge1::Fn<
          void(rust::cxxbridge1::Box<brave_rewards::HttpRoundtripContext>,
               brave_rewards::HttpResponse)> callback,
      rust::cxxbridge1::Box<brave_rewards::HttpRoundtripContext> ctx);

 private:
  scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory_;
  std::unique_ptr<network::SimpleURLLoader> sku_sdk_loader_;

  const net::NetworkTrafficAnnotationTag& GetNetworkTrafficAnnotationTag() {
    static const net::NetworkTrafficAnnotationTag
        network_traffic_annotation_tag =
            net::DefineNetworkTrafficAnnotation("sku_sdk_execute_request", R"(
        semantics {
          sender: "Brave SKU SDK"
          description:
            "Call the SKU SDK implementation provided by the caller"
          trigger:
            "Any Brave webpage using SKU SDK where window.brave.sku.*"
            "methods are called; ex: fetch_order / fetch_order_credentials"
          data: "JSON data comprising an order."
          destination: OTHER
          destination_other: "Brave developers"
        }
        policy {
          cookies_allowed: NO
        })");
    return network_traffic_annotation_tag;
  }

  void OnFetchComplete(
      rust::cxxbridge1::Fn<
          void(rust::cxxbridge1::Box<brave_rewards::HttpRoundtripContext>,
               brave_rewards::HttpResponse)> callback,
      rust::cxxbridge1::Box<brave_rewards::HttpRoundtripContext> ctx,
      std::unique_ptr<std::string> response_body);
};

std::unique_ptr<SkusSdkFetcher> fetcher;

SkusSdkFetcher::SkusSdkFetcher(
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory)
    : url_loader_factory_(url_loader_factory) {}

SkusSdkFetcher::~SkusSdkFetcher() {}

void SkusSdkFetcher::BeginFetch(
    const brave_rewards::HttpRequest& req,
    rust::cxxbridge1::Fn<
        void(rust::cxxbridge1::Box<brave_rewards::HttpRoundtripContext>,
             brave_rewards::HttpResponse)> callback,
    rust::cxxbridge1::Box<brave_rewards::HttpRoundtripContext> ctx) {
  auto resource_request = std::make_unique<network::ResourceRequest>();
  resource_request->url = GURL(ruststring_2_stdstring(req.url));
  resource_request->method = ruststring_2_stdstring(req.method).c_str();
  resource_request->credentials_mode = network::mojom::CredentialsMode::kOmit;
  // No cache read, always download from the network.
  resource_request->load_flags =
      net::LOAD_BYPASS_CACHE | net::LOAD_DISABLE_CACHE;

  for (size_t i = 0; i < req.headers.size(); i++) {
    resource_request->headers.AddHeaderFromString(
        ruststring_2_stdstring(req.headers[i]));
  }

  sku_sdk_loader_ = network::SimpleURLLoader::Create(
      std::move(resource_request), GetNetworkTrafficAnnotationTag());

  sku_sdk_loader_->DownloadToString(
      url_loader_factory_.get(),
      base::BindOnce(&SkusSdkFetcher::OnFetchComplete, base::Unretained(this),
                     std::move(callback), std::move(ctx)),
      kMaxResponseSize);
}

void SkusSdkFetcher::OnFetchComplete(
    rust::cxxbridge1::Fn<
        void(rust::cxxbridge1::Box<brave_rewards::HttpRoundtripContext>,
             brave_rewards::HttpResponse)> callback,
    rust::cxxbridge1::Box<brave_rewards::HttpRoundtripContext> ctx,
    std::unique_ptr<std::string> response_body) {
  if (!response_body) {
    std::vector<uint8_t> body_bytes;
    brave_rewards::HttpResponse resp = {
        brave_rewards::RewardsResult::RequestFailed,
        500,
        {},
        body_bytes,
    };
    callback(std::move(ctx), resp);
    return;
  }

  std::vector<uint8_t> body_bytes(response_body->begin(), response_body->end());

  brave_rewards::HttpResponse resp = {
      brave_rewards::RewardsResult::Ok,
      200,
      {},
      body_bytes,
  };

  callback(std::move(ctx), resp);
}

void OnRefreshOrder(brave_rewards::RefreshOrderCallbackState* callback_state,
                    brave_rewards::RewardsResult result,
                    rust::cxxbridge1::Str order) {
  std::string order_str = ruststr_2_stdstring(order);
  if (callback_state->cb) {
    std::move(callback_state->cb).Run(order_str);
  }
  delete callback_state;
}

// std::string GetEnvironment() {
//   // TODO(bsclifton): implement similar to logic to
//   // https://github.com/brave/brave-core/pull/10358/files#diff-2170e2d6e88ab6e0202eac0280482f5a45f468d0fcc8d9d4d48fc358812b4a0cR35
//   return "development";
// }

void OnScheduleWakeup(rust::cxxbridge1::Fn<void()> done) {
  done();
}

}  // namespace


namespace brave_rewards {

brave_rewards::StupidDictionary* g_dictionary = NULL;

StupidDictionary::StupidDictionary() {}
StupidDictionary::~StupidDictionary() {}

//static lol
StupidDictionary* StupidDictionary::GetInstance() {
  if (g_dictionary) return g_dictionary;
  g_dictionary = new StupidDictionary();
  return g_dictionary;
}

void shim_purge() {
  LOG(ERROR) << "shim_purge";
  ::prefs::ScopedDictionaryPrefUpdate update(g_SkusSdk->prefs_,
                                             prefs::kSkusDictionary);
  std::unique_ptr<::prefs::DictionaryValueUpdate> dictionary = update.Get();
  DCHECK(dictionary);
  dictionary->Clear();
}

void shim_set(rust::cxxbridge1::Str key, rust::cxxbridge1::Str value) {
  std::string key_string = ruststr_2_stdstring(key);
  std::string value_string = ruststr_2_stdstring(value);
  LOG(ERROR) << "shim_set: `" << key_string << "` = `" << value_string << "`";

  ::prefs::ScopedDictionaryPrefUpdate update(g_SkusSdk->prefs_,
                                             prefs::kSkusDictionary);
  std::unique_ptr<::prefs::DictionaryValueUpdate> dictionary = update.Get();
  DCHECK(dictionary);
  dictionary->SetString(key_string, value_string);
}

const std::string& shim_get(rust::cxxbridge1::Str key) {
  std::string key_string = ruststr_2_stdstring(key);
  LOG(ERROR) << "shim_get: `" << key_string << "`";

  auto* stupid = StupidDictionary::GetInstance();

  const base::Value* dictionary =
      g_SkusSdk->prefs_->GetDictionary(prefs::kSkusDictionary);
  DCHECK(dictionary);
  DCHECK(dictionary->is_dict());
  const base::Value* value = dictionary->FindKey(key_string);
  if (value) {
    stupid->dictionary_[key_string] = value->GetString();
    return stupid->dictionary_[key_string];
  }
  stupid->dictionary_[key_string] = "";
  return stupid->dictionary_[key_string];
}

void shim_scheduleWakeup(::std::uint64_t delay_ms,
                         rust::cxxbridge1::Fn<void()> done) {
  LOG(ERROR) << "shim_scheduleWakeup " << delay_ms;
  base::SequencedTaskRunnerHandle::Get()->PostDelayedTask(
      FROM_HERE, base::BindOnce(&OnScheduleWakeup, std::move(done)),
      base::TimeDelta::FromMilliseconds(delay_ms));
}

void shim_executeRequest(
    const brave_rewards::HttpRequest& req,
    rust::cxxbridge1::Fn<
        void(rust::cxxbridge1::Box<brave_rewards::HttpRoundtripContext>,
             brave_rewards::HttpResponse)> done,
    rust::cxxbridge1::Box<brave_rewards::HttpRoundtripContext> ctx) {
  fetcher = std::make_unique<SkusSdkFetcher>(g_SkusSdk->url_loader_factory_);
  fetcher->BeginFetch(req, std::move(done), std::move(ctx));
}

// static
void SkusSdkImpl::RegisterProfilePrefs(
    user_prefs::PrefRegistrySyncable* registry) {
  registry->RegisterDictionaryPref(prefs::kSkusDictionary);
  registry->RegisterStringPref(prefs::kSkusVPNCredential, "");
}

SkusSdkImpl::SkusSdkImpl(
    PrefService* prefs,
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory)
    : url_loader_factory_(url_loader_factory), prefs_(prefs) {
  g_SkusSdk = this;
}

SkusSdkImpl::~SkusSdkImpl() {}

void SkusSdkImpl::RefreshOrder(const std::string& order_id,
                               RefreshOrderCallback callback) {
  ::rust::Box<CppSDK> sdk = initialize_sdk("development");

  std::unique_ptr<RefreshOrderCallbackState> cbs(new RefreshOrderCallbackState);
  cbs->cb = std::move(callback);

  sdk->refresh_order(OnRefreshOrder, std::move(cbs), order_id.c_str());
}

void SkusSdkImpl::FetchOrderCredentials(const std::string& order_id) {
  ::rust::Box<CppSDK> sdk = initialize_sdk("development");

  // TODO(bsclifton): fill me in

  // sdk->fetch_order_credentials(on_refresh_order, std::move(cbs),
  // order_id.c_str());
}

}  // namespace brave_rewards
