/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_WALLET_BROWSER_KEYRING_CONTROLLER_H_
#define BRAVE_COMPONENTS_BRAVE_WALLET_BROWSER_KEYRING_CONTROLLER_H_

#include <memory>
#include <string>
#include <vector>

#include "base/gtest_prod_util.h"
#include "base/values.h"
#include "brave/components/brave_wallet/browser/brave_wallet_types.h"
#include "brave/components/brave_wallet/browser/password_encryptor.h"
#include "brave/components/brave_wallet/common/brave_wallet.mojom.h"
#include "components/keyed_service/core/keyed_service.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "mojo/public/cpp/bindings/receiver_set.h"
#include "mojo/public/cpp/bindings/remote_set.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

class PrefChangeRegistrar;
class PrefService;

namespace base {
class OneShotTimer;
}

namespace brave_wallet {

class HDKeyring;
class EthTransaction;
class KeyringControllerUnitTest;
class BraveWalletProviderImplUnitTest;

// This class is not thread-safe and should have single owner
class KeyringController : public KeyedService, public mojom::KeyringController {
 public:
  explicit KeyringController(PrefService* prefs);
  ~KeyringController() override;

  static void MigrateObsoleteProfilePrefs(PrefService* prefs);

  static bool HasPrefForKeyring(PrefService* prefs,
                                const std::string& key,
                                const std::string& id);
  static const base::Value* GetPrefForKeyring(PrefService* prefs,
                                              const std::string& key,
                                              const std::string& id);
  static base::Value* GetPrefForHardwareKeyringUpdate(PrefService* prefs);
  static base::Value* GetPrefForKeyringUpdate(PrefService* prefs,
                                              const std::string& key,
                                              const std::string& id);
  // If keyring dicionary for id doesn't exist, it will be created.
  static void SetPrefForKeyring(PrefService* prefs,
                                const std::string& key,
                                base::Value value,
                                const std::string& id);

  // Account path will be used as key in kAccountMetas
  static void SetAccountMetaForKeyring(
      PrefService* prefs,
      const std::string& account_path,
      const absl::optional<std::string> name,
      const absl::optional<std::string> address,
      const std::string& id);

  static std::string GetAccountNameForKeyring(PrefService* prefs,
                                              const std::string& account_path,
                                              const std::string& id);
  static std::string GetAccountAddressForKeyring(
      PrefService* prefs,
      const std::string& account_path,
      const std::string& id);

  static std::string GetAccountPathByIndex(size_t index);

  struct ImportedAccountInfo {
    std::string account_name;
    std::string account_address;
    std::string encrypted_private_key;
  };
  static void SetImportedAccountForKeyring(PrefService* prefs,
                                           const ImportedAccountInfo& info,
                                           const std::string& id);
  static std::vector<ImportedAccountInfo> GetImportedAccountsForKeyring(
      PrefService* prefs,
      const std::string& id);
  static void RemoveImportedAccountForKeyring(PrefService* prefs,
                                              const std::string& address,
                                              const std::string& id);

  mojo::PendingRemote<mojom::KeyringController> MakeRemote();
  void Bind(mojo::PendingReceiver<mojom::KeyringController> receiver);

  // mojom::KeyringController
  // Must unlock before using this API otherwise it will return empty string
  void GetMnemonicForDefaultKeyring(
      GetMnemonicForDefaultKeyringCallback callback) override;
  void CreateWallet(const std::string& password,
                    CreateWalletCallback callback) override;
  void RestoreWallet(const std::string& mnemonic,
                     const std::string& password,
                     bool is_legacy_brave_wallet,
                     RestoreWalletCallback callback) override;
  void Unlock(const std::string& password, UnlockCallback callback) override;
  void Lock() override;
  void IsLocked(IsLockedCallback callback) override;
  void AddAccount(const std::string& account_name,
                  AddAccountCallback callback) override;
  void GetPrivateKeyForDefaultKeyringAccount(
      const std::string& address,
      GetPrivateKeyForDefaultKeyringAccountCallback callback) override;
  void ImportAccount(const std::string& account_name,
                     const std::string& private_key,
                     ImportAccountCallback callback) override;
  void ImportAccountFromJson(const std::string& account_name,
                             const std::string& password,
                             const std::string& json,
                             ImportAccountCallback callback) override;
  void AddHardwareAccounts(
      std::vector<mojom::HardwareWalletAccountPtr> info) override;
  void GetHardwareAccounts(GetHardwareAccountsCallback callback) override;
  void RemoveHardwareAccount(const std::string& address) override;
  void GetPrivateKeyForImportedAccount(
      const std::string& address,
      GetPrivateKeyForImportedAccountCallback callback) override;
  void RemoveImportedAccount(const std::string& address,
                             RemoveImportedAccountCallback callback) override;
  void IsWalletBackedUp(IsWalletBackedUpCallback callback) override;
  void NotifyWalletBackupComplete() override;
  void GetDefaultKeyringInfo(GetDefaultKeyringInfoCallback callback) override;
  void Reset() override;
  void SetDefaultKeyringDerivedAccountName(
      const std::string& address,
      const std::string& name,
      SetDefaultKeyringDerivedAccountNameCallback callback) override;
  void SetDefaultKeyringImportedAccountName(
      const std::string& address,
      const std::string& name,
      SetDefaultKeyringImportedAccountNameCallback callback) override;

  bool IsDefaultKeyringCreated();

  void SignTransactionByDefaultKeyring(const std::string& address,
                                       EthTransaction* tx,
                                       uint256_t chain_id);

  struct SignatureWithError {
    SignatureWithError();
    SignatureWithError(SignatureWithError&& other);
    SignatureWithError& operator=(SignatureWithError&& other);
    SignatureWithError(const SignatureWithError&) = delete;
    SignatureWithError& operator=(const SignatureWithError&) = delete;
    ~SignatureWithError();

    absl::optional<std::vector<uint8_t>> signature;
    std::string error_message;
  };
  SignatureWithError SignMessageByDefaultKeyring(
      const std::string& address,
      const std::vector<uint8_t>& message);

  void AddAccountsWithDefaultName(size_t number);

  bool IsLocked() const;

  void AddObserver(::mojo::PendingRemote<mojom::KeyringControllerObserver>
                       observer) override;
  void NotifyUserInteraction() override;
  void GetSelectedAccount(GetSelectedAccountCallback callback) override;
  void SetSelectedAccount(const std::string& address,
                          SetSelectedAccountCallback callback) override;
  void GetAutoLockMinutes(GetAutoLockMinutesCallback callback) override;
  void SetAutoLockMinutes(int32_t minutes,
                          SetAutoLockMinutesCallback callback) override;

  /* TODO(darkdh): For other keyrings support
  void DeleteKeyring(size_t index);
  HDKeyring* GetKeyring(size_t index);
  */

 private:
  FRIEND_TEST_ALL_PREFIXES(KeyringControllerUnitTest, GetPrefInBytesForKeyring);
  FRIEND_TEST_ALL_PREFIXES(KeyringControllerUnitTest, SetPrefInBytesForKeyring);
  FRIEND_TEST_ALL_PREFIXES(KeyringControllerUnitTest,
                           GetOrCreateNonceForKeyring);
  FRIEND_TEST_ALL_PREFIXES(KeyringControllerUnitTest,
                           CreateEncryptorForKeyring);
  FRIEND_TEST_ALL_PREFIXES(KeyringControllerUnitTest, CreateDefaultKeyring);
  FRIEND_TEST_ALL_PREFIXES(KeyringControllerUnitTest,
                           CreateDefaultKeyringInternal);
  FRIEND_TEST_ALL_PREFIXES(KeyringControllerUnitTest, RestoreDefaultKeyring);
  FRIEND_TEST_ALL_PREFIXES(KeyringControllerUnitTest,
                           UnlockResumesDefaultKeyring);
  FRIEND_TEST_ALL_PREFIXES(KeyringControllerUnitTest,
                           GetMnemonicForDefaultKeyring);
  FRIEND_TEST_ALL_PREFIXES(KeyringControllerUnitTest, LockAndUnlock);
  FRIEND_TEST_ALL_PREFIXES(KeyringControllerUnitTest, Reset);
  FRIEND_TEST_ALL_PREFIXES(KeyringControllerUnitTest, AccountMetasForKeyring);
  FRIEND_TEST_ALL_PREFIXES(KeyringControllerUnitTest, CreateAndRestoreWallet);
  FRIEND_TEST_ALL_PREFIXES(KeyringControllerUnitTest, AddAccount);
  FRIEND_TEST_ALL_PREFIXES(KeyringControllerUnitTest, ImportedAccounts);
  FRIEND_TEST_ALL_PREFIXES(KeyringControllerUnitTest,
                           GetPrivateKeyForDefaultKeyringAccount);
  FRIEND_TEST_ALL_PREFIXES(KeyringControllerUnitTest,
                           SetDefaultKeyringDerivedAccountMeta);
  FRIEND_TEST_ALL_PREFIXES(KeyringControllerUnitTest, RestoreLegacyBraveWallet);
  FRIEND_TEST_ALL_PREFIXES(KeyringControllerUnitTest, AutoLock);
  FRIEND_TEST_ALL_PREFIXES(KeyringControllerUnitTest, SetSelectedAccount);
  friend class BraveWalletProviderImplUnitTest;
  friend class EthTxControllerUnitTest;

  void AddAccountForDefaultKeyring(const std::string& account_name);
  void OnAutoLockFired();
  std::vector<mojom::AccountInfoPtr> GetHardwareAccountsSync();

  // Address will be returned when success
  absl::optional<std::string> ImportAccountForDefaultKeyring(
      const std::string& account_name,
      const std::vector<uint8_t>& private_key);

  size_t GetAccountMetasNumberForKeyring(const std::string& id);

  std::vector<mojom::AccountInfoPtr> GetAccountInfosForKeyring(
      const std::string& id);

  const std::string GetMnemonicForDefaultKeyringImpl();

  bool GetPrefInBytesForKeyring(const std::string& key,
                                std::vector<uint8_t>* bytes,
                                const std::string& id) const;
  void SetPrefInBytesForKeyring(const std::string& key,
                                base::span<const uint8_t> bytes,
                                const std::string& id);
  std::vector<uint8_t> GetOrCreateNonceForKeyring(const std::string& id);
  bool CreateEncryptorForKeyring(const std::string& password,
                                 const std::string& id);
  bool CreateDefaultKeyringInternal(const std::string& mnemonic,
                                    bool is_legacy_brave_wallet);

  // Currently only support one default keyring, `CreateDefaultKeyring` and
  // `RestoreDefaultKeyring` will overwrite existing one if success
  HDKeyring* CreateDefaultKeyring(const std::string& password);
  // Restore default keyring from backup seed phrase
  HDKeyring* RestoreDefaultKeyring(const std::string& mnemonic,
                                   const std::string& password,
                                   bool is_legacy_brave_wallet);
  // It's used to reconstruct same default keyring between browser relaunch
  HDKeyring* ResumeDefaultKeyring(const std::string& password);

  void NotifyAccountsChanged();
  void StopAutoLockTimer();
  void ResetAutoLockTimer();
  void OnAutoLockPreferenceChanged();
  void OnSelectedAccountPreferenceChanged();

  std::unique_ptr<PasswordEncryptor> encryptor_;
  std::unique_ptr<HDKeyring> default_keyring_;
  std::unique_ptr<base::OneShotTimer> auto_lock_timer_;
  std::unique_ptr<PrefChangeRegistrar> pref_change_registrar_;

  // TODO(darkdh): For other keyrings support
  // std::vector<std::unique_ptr<HDKeyring>> keyrings_;

  PrefService* prefs_;

  mojo::RemoteSet<mojom::KeyringControllerObserver> observers_;
  mojo::ReceiverSet<mojom::KeyringController> receivers_;

  KeyringController(const KeyringController&) = delete;
  KeyringController& operator=(const KeyringController&) = delete;
};

}  // namespace brave_wallet

#endif  // BRAVE_COMPONENTS_BRAVE_WALLET_BROWSER_KEYRING_CONTROLLER_H_
