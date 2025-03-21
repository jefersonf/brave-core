/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_WALLET_BROWSER_BRAVE_WALLET_SERVICE_H_
#define BRAVE_COMPONENTS_BRAVE_WALLET_BROWSER_BRAVE_WALLET_SERVICE_H_

#include <memory>
#include <string>

#include "base/containers/queue.h"
#include "base/gtest_prod_util.h"
#include "base/memory/weak_ptr.h"
#include "base/time/time.h"
#include "brave/components/brave_wallet/browser/brave_wallet_service_delegate.h"
#include "brave/components/brave_wallet/common/brave_wallet.mojom.h"
#include "components/keyed_service/core/keyed_service.h"
#include "components/prefs/pref_change_registrar.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "mojo/public/cpp/bindings/receiver_set.h"
#include "mojo/public/cpp/bindings/remote_set.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

class PrefService;

namespace brave_wallet {

constexpr char kBraveWalletDailyHistogramName[] = "Brave.Wallet.UsageDaily";
constexpr char kBraveWalletWeeklyHistogramName[] = "Brave.Wallet.UsageWeekly";
constexpr char kBraveWalletMonthlyHistogramName[] = "Brave.Wallet.UsageMonthly";

class KeyringController;

class BraveWalletService : public KeyedService,
                           public mojom::BraveWalletService,
                           public BraveWalletServiceDelegate::Observer {
 public:
  explicit BraveWalletService(
      std::unique_ptr<BraveWalletServiceDelegate> delegate,
      KeyringController* keyring_controller,
      PrefService* prefs);
  ~BraveWalletService() override;

  BraveWalletService(const BraveWalletService&) = delete;
  BraveWalletService& operator=(const BraveWalletService&) = delete;

  mojo::PendingRemote<mojom::BraveWalletService> MakeRemote();
  void Bind(mojo::PendingReceiver<mojom::BraveWalletService> receiver);

  static void MigrateUserAssetEthContractAddress(PrefService* prefs);

  // mojom::BraveWalletService:
  void AddObserver(::mojo::PendingRemote<mojom::BraveWalletServiceObserver>
                       observer) override;

  void GetUserAssets(const std::string& chain_id,
                     GetUserAssetsCallback callback) override;
  void AddUserAsset(mojom::ERCTokenPtr token,
                    const std::string& chain_id,
                    AddUserAssetCallback callback) override;
  void RemoveUserAsset(mojom::ERCTokenPtr token,
                       const std::string& chain_id,
                       RemoveUserAssetCallback callback) override;
  void SetUserAssetVisible(mojom::ERCTokenPtr token,
                           const std::string& chain_id,
                           bool visible,
                           SetUserAssetVisibleCallback callback) override;
  void IsCryptoWalletsInstalled(
      IsCryptoWalletsInstalledCallback callback) override;
  void IsMetaMaskInstalled(IsMetaMaskInstalledCallback callback) override;
  void ImportFromCryptoWallets(
      const std::string& password,
      const std::string& new_password,
      ImportFromCryptoWalletsCallback callback) override;
  void ImportFromMetaMask(const std::string& password,
                          const std::string& new_password,
                          ImportFromMetaMaskCallback callback) override;
  void GetDefaultWallet(GetDefaultWalletCallback callback) override;
  void SetDefaultWallet(mojom::DefaultWallet default_wallet) override;
  void HasEthereumPermission(const std::string& origin,
                             const std::string& account,
                             HasEthereumPermissionCallback callback) override;
  void ResetEthereumPermission(
      const std::string& origin,
      const std::string& account,
      ResetEthereumPermissionCallback callback) override;
  void GetActiveOrigin(GetActiveOriginCallback callback) override;
  void GetPendingSignMessageRequest(
      GetPendingSignMessageRequestCallback callback) override;
  void NotifySignMessageRequestProcessed(bool approved, int id) override;

  // BraveWalletServiceDelegate::Observer:
  void OnActiveOriginChanged(const std::string& origin) override;

  void RecordWalletUsage(base::Time wallet_last_used);

  struct SignMessageRequest {
    int id;
    std::string address;
    std::string message;
  };
  void AddSignMessageRequest(SignMessageRequest&& request,
                             base::OnceCallback<void(bool)> callback);

 private:
  void OnDefaultWalletChanged();

  friend class BraveWalletProviderImplUnitTest;

  FRIEND_TEST_ALL_PREFIXES(BraveWalletServiceUnitTest, GetChecksumAddress);
  FRIEND_TEST_ALL_PREFIXES(BraveWalletServiceUnitTest, OnGetImportInfo);

  absl::optional<std::string> GetChecksumAddress(
      const std::string& contract_address,
      const std::string& chain_id);
  void OnWalletUnlockPreferenceChanged(const std::string& pref_name);
  void OnP3ATimerFired();

  void OnGetImportInfo(const std::string& new_password,
                       base::OnceCallback<void(bool)> callback,
                       bool result,
                       BraveWalletServiceDelegate::ImportInfo info);

  base::queue<SignMessageRequest> sign_message_requests_;
  base::queue<base::OnceCallback<void(bool)>> sign_message_callbacks_;
  mojo::RemoteSet<mojom::BraveWalletServiceObserver> observers_;
  std::unique_ptr<BraveWalletServiceDelegate> delegate_;
  KeyringController* keyring_controller_;
  PrefService* prefs_;
  mojo::ReceiverSet<mojom::BraveWalletService> receivers_;
  PrefChangeRegistrar pref_change_registrar_;
  base::RepeatingTimer p3a_periodic_timer_;
  base::WeakPtrFactory<BraveWalletService> weak_ptr_factory_;
};

}  // namespace brave_wallet

#endif  // BRAVE_COMPONENTS_BRAVE_WALLET_BROWSER_BRAVE_WALLET_SERVICE_H_
