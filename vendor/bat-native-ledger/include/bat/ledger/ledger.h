/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_LEDGER_INCLUDE_BAT_LEDGER_LEDGER_H_
#define BRAVE_VENDOR_BAT_NATIVE_LEDGER_INCLUDE_BAT_LEDGER_LEDGER_H_

#include <stdint.h>

#include <memory>
#include <string>
#include <vector>

#include "base/containers/flat_map.h"
#include "bat/ledger/export.h"
#include "bat/ledger/mojom_structs.h"
#include "bat/ledger/ledger_client.h"

namespace ledger {

extern type::Environment _environment;
extern bool is_debug;
extern bool is_testing;
extern int reconcile_interval;  // minutes
extern int retry_interval;      // seconds

using PublisherBannerCallback = std::function<void(type::PublisherBannerPtr)>;

using GetRewardsParametersCallback =
    std::function<void(type::RewardsParametersPtr)>;

using OnRefreshPublisherCallback = std::function<void(type::PublisherStatus)>;

using HasSufficientBalanceToReconcileCallback = std::function<void(bool)>;

using FetchBalanceCallback =
    std::function<void(type::Result, type::BalancePtr)>;

using ExternalWalletCallback =
    std::function<void(type::Result, type::ExternalWalletPtr)>;

using ExternalWalletAuthorizationCallback =
    std::function<void(type::Result, base::flat_map<std::string, std::string>)>;

using FetchPromotionCallback =
    std::function<void(type::Result, type::PromotionList)>;

using ClaimPromotionCallback =
    std::function<void(type::Result, const std::string&)>;

using RewardsInternalsInfoCallback =
    std::function<void(type::RewardsInternalsInfoPtr)>;

using AttestPromotionCallback =
    std::function<void(type::Result, type::PromotionPtr)>;

using GetBalanceReportCallback =
    std::function<void(type::Result, type::BalanceReportInfoPtr)>;

using GetBalanceReportListCallback =
    std::function<void(type::BalanceReportInfoList)>;

using ContributionInfoListCallback =
    std::function<void(type::ContributionInfoList)>;

using GetMonthlyReportCallback =
    std::function<void(type::Result, type::MonthlyReportInfoPtr)>;

using GetAllMonthlyReportIdsCallback =
    std::function<void(const std::vector<std::string>&)>;

using GetEventLogsCallback = std::function<void(type::EventLogs)>;

using SKUOrderCallback = std::function<void(type::Result, const std::string&)>;

using GetContributionReportCallback =
    std::function<void(type::ContributionReportInfoList)>;

using GetTransactionReportCallback =
    std::function<void(type::TransactionReportInfoList)>;

using GetAllPromotionsCallback = std::function<void(type::PromotionMap)>;

using ResultCallback = std::function<void(const type::Result)>;

using PendingContributionsTotalCallback = std::function<void(double)>;

using PendingContributionInfoListCallback =
    std::function<void(type::PendingContributionInfoList)>;

using UnverifiedPublishersCallback =
    std::function<void(std::vector<std::string>&&)>;

using PublisherInfoListCallback = std::function<void(type::PublisherInfoList)>;

using PublisherInfoCallback =
    std::function<void(type::Result, type::PublisherInfoPtr)>;

using GetPublisherInfoCallback =
    std::function<void(type::Result, type::PublisherInfoPtr)>;

using GetBraveWalletCallback = std::function<void(type::BraveWalletPtr)>;

using GetTransferableAmountCallback = std::function<void(double)>;

using PostSuggestionsClaimCallback =
    std::function<void(type::Result result, std::string drain_id)>;

using GetDrainCallback =
    std::function<void(type::Result result, type::DrainStatus status)>;

class LEDGER_EXPORT Ledger {
 public:
  static bool IsMediaLink(const std::string& url,
                          const std::string& first_party_url,
                          const std::string& referrer);

  Ledger() = default;
  virtual ~Ledger() = default;

  Ledger(const Ledger&) = delete;
  Ledger& operator=(const Ledger&) = delete;

  static Ledger* CreateInstance(LedgerClient* client);

  virtual void Initialize(bool execute_create_script, ResultCallback) = 0;

  virtual void CreateWallet(ResultCallback callback) = 0;

  virtual void OneTimeTip(const std::string& publisher_key,
                          double amount,
                          ResultCallback callback) = 0;

  virtual void OnLoad(type::VisitDataPtr visit_data, uint64_t current_time) = 0;

  virtual void OnUnload(uint32_t tab_id, uint64_t current_time) = 0;

  virtual void OnShow(uint32_t tab_id, uint64_t current_time) = 0;

  virtual void OnHide(uint32_t tab_id, uint64_t current_time) = 0;

  virtual void OnForeground(uint32_t tab_id, uint64_t current_time) = 0;

  virtual void OnBackground(uint32_t tab_id, uint64_t current_time) = 0;

  virtual void OnXHRLoad(uint32_t tab_id,
                         const std::string& url,
                         const base::flat_map<std::string, std::string>& parts,
                         const std::string& first_party_url,
                         const std::string& referrer,
                         type::VisitDataPtr visit_data) = 0;

  virtual void OnPostData(const std::string& url,
                          const std::string& first_party_url,
                          const std::string& referrer,
                          const std::string& post_data,
                          type::VisitDataPtr visit_data) = 0;

  virtual void GetActivityInfoList(uint32_t start,
                                   uint32_t limit,
                                   type::ActivityInfoFilterPtr filter,
                                   PublisherInfoListCallback callback) = 0;

  virtual void GetExcludedList(PublisherInfoListCallback callback) = 0;

  virtual void SetPublisherMinVisitTime(int duration_in_seconds) = 0;

  virtual void SetPublisherMinVisits(int visits) = 0;

  virtual void SetPublisherAllowNonVerified(bool allow) = 0;

  virtual void SetPublisherAllowVideos(bool allow) = 0;

  virtual void SetAutoContributionAmount(double amount) = 0;

  virtual void SetAutoContributeEnabled(bool enabled) = 0;

  virtual uint64_t GetReconcileStamp() = 0;

  virtual int GetPublisherMinVisitTime() = 0;  // In milliseconds

  virtual int GetPublisherMinVisits() = 0;

  virtual bool GetPublisherAllowNonVerified() = 0;

  virtual bool GetPublisherAllowVideos() = 0;

  virtual double GetAutoContributionAmount() = 0;

  virtual bool GetAutoContributeEnabled() = 0;

  virtual void GetRewardsParameters(GetRewardsParametersCallback callback) = 0;

  virtual void FetchPromotions(FetchPromotionCallback callback) = 0;

  // |payload|:
  // desktop and Android: empty
  // iOS: { "publicKey": "{{publicKey}}" }
  // =====================================
  // |callback| returns result as json
  // desktop: { "captchaImage": "{{captchaImage}}", "hint": "{{hint}}" }
  // iOS and Android: { "nonce": "{{nonce}}" }
  virtual void ClaimPromotion(const std::string& promotion_id,
                              const std::string& payload,
                              ClaimPromotionCallback callback) = 0;

  // |solution|:
  // desktop:
  //  {
  //    "captchaId": "{{captchaId}}",
  //    "x": "{{x}}",
  //    "y": "{{y}}"
  //  }
  // iOS:
  //  {
  //    "nonce": "{{nonce}}",
  //    "blob": "{{blob}}",
  //    "signature": "{{signature}}"
  //  }
  // android:
  //  {
  //    "nonce": "{{nonce}}",
  //    "token": "{{token}}"
  //  }
  virtual void AttestPromotion(const std::string& promotion_id,
                               const std::string& solution,
                               AttestPromotionCallback callback) = 0;

  virtual void GetBalanceReport(type::ActivityMonth month,
                                int year,
                                GetBalanceReportCallback callback) = 0;

  virtual void GetAllBalanceReports(GetBalanceReportListCallback callback) = 0;

  virtual type::AutoContributePropertiesPtr GetAutoContributeProperties() = 0;

  virtual void RecoverWallet(const std::string& pass_phrase,
                             ResultCallback callback) = 0;

  virtual void SetPublisherExclude(const std::string& publisher_id,
                                   type::PublisherExclude exclude,
                                   ResultCallback callback) = 0;

  virtual void RestorePublishers(ResultCallback callback) = 0;

  virtual void GetPublisherActivityFromUrl(
      uint64_t window_id,
      type::VisitDataPtr visit_data,
      const std::string& publisher_blob) = 0;

  virtual void GetPublisherBanner(const std::string& publisher_id,
                                  PublisherBannerCallback callback) = 0;

  virtual void RemoveRecurringTip(const std::string& publisher_key,
                                  ResultCallback callback) = 0;

  virtual uint64_t GetCreationStamp() = 0;

  virtual void HasSufficientBalanceToReconcile(
      HasSufficientBalanceToReconcileCallback callback) = 0;

  virtual void GetRewardsInternalsInfo(
      RewardsInternalsInfoCallback callback) = 0;

  virtual void SaveRecurringTip(type::RecurringTipPtr info,
                                ResultCallback callback) = 0;

  virtual void GetRecurringTips(PublisherInfoListCallback callback) = 0;

  virtual void GetOneTimeTips(PublisherInfoListCallback callback) = 0;

  virtual void RefreshPublisher(const std::string& publisher_key,
                                OnRefreshPublisherCallback callback) = 0;

  virtual void StartMonthlyContribution() = 0;

  virtual void SaveMediaInfo(
      const std::string& type,
      const base::flat_map<std::string, std::string>& data,
      PublisherInfoCallback callback) = 0;

  virtual void UpdateMediaDuration(uint64_t window_id,
                                   const std::string& publisher_key,
                                   uint64_t duration,
                                   bool first_visit) = 0;

  virtual void GetPublisherInfo(const std::string& publisher_key,
                                GetPublisherInfoCallback callback) = 0;

  virtual void GetPublisherPanelInfo(const std::string& publisher_key,
                                     GetPublisherInfoCallback callback) = 0;

  virtual void SavePublisherInfo(uint64_t window_id,
                                 type::PublisherInfoPtr publisher_info,
                                 ResultCallback callback) = 0;

  virtual void SetInlineTippingPlatformEnabled(
      type::InlineTipsPlatforms platform,
      bool enabled) = 0;

  virtual bool GetInlineTippingPlatformEnabled(
      type::InlineTipsPlatforms platform) = 0;

  virtual std::string GetShareURL(
      const base::flat_map<std::string, std::string>& args) = 0;

  virtual void GetPendingContributions(
      PendingContributionInfoListCallback callback) = 0;

  virtual void RemovePendingContribution(uint64_t id,
                                         ResultCallback callback) = 0;

  virtual void RemoveAllPendingContributions(ResultCallback callback) = 0;

  virtual void GetPendingContributionsTotal(
      PendingContributionsTotalCallback callback) = 0;

  virtual void FetchBalance(FetchBalanceCallback callback) = 0;

  virtual void GetExternalWallet(const std::string& wallet_type,
                                 ExternalWalletCallback callback) = 0;

  virtual void ExternalWalletAuthorization(
      const std::string& wallet_type,
      const base::flat_map<std::string, std::string>& args,
      ExternalWalletAuthorizationCallback callback) = 0;

  virtual void DisconnectWallet(const std::string& wallet_type,
                                ResultCallback callback) = 0;

  virtual void GetAllPromotions(GetAllPromotionsCallback callback) = 0;

  virtual void GetAnonWalletStatus(ResultCallback callback) = 0;

  virtual void GetTransactionReport(type::ActivityMonth month,
                                    int year,
                                    GetTransactionReportCallback callback) = 0;

  virtual void GetContributionReport(
      type::ActivityMonth month,
      int year,
      GetContributionReportCallback callback) = 0;

  virtual void GetAllContributions(ContributionInfoListCallback callback) = 0;

  virtual void SavePublisherInfoForTip(type::PublisherInfoPtr info,
                                       ResultCallback callback) = 0;

  virtual void GetMonthlyReport(type::ActivityMonth month,
                                int year,
                                GetMonthlyReportCallback callback) = 0;

  virtual void GetAllMonthlyReportIds(
      GetAllMonthlyReportIdsCallback callback) = 0;

  virtual void ProcessSKU(const std::vector<type::SKUOrderItem>& items,
                          const std::string& wallet_type,
                          SKUOrderCallback callback) = 0;

  virtual void Shutdown(ResultCallback callback) = 0;

  virtual void GetEventLogs(GetEventLogsCallback callback) = 0;

  virtual void GetBraveWallet(GetBraveWalletCallback callback) = 0;

  virtual std::string GetWalletPassphrase() = 0;

  virtual void LinkBraveWallet(const std::string& destination_payment_id,
                               PostSuggestionsClaimCallback callback) = 0;

  virtual void GetDrainStatus(const std::string& drain_id,
                              GetDrainCallback callback) = 0;

  virtual void GetTransferableAmount(
      GetTransferableAmountCallback callback) = 0;
};

}  // namespace ledger

#endif  // BRAVE_VENDOR_BAT_NATIVE_LEDGER_INCLUDE_BAT_LEDGER_LEDGER_H_
