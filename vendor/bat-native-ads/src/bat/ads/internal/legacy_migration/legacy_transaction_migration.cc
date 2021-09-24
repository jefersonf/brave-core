/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/legacy_migration/legacy_transaction_migration.h"

#include <string>

#include "base/check.h"
#include "base/json/json_reader.h"
#include "base/strings/string_number_conversions.h"
#include "base/values.h"
#include "bat/ads/ads_client.h"
#include "bat/ads/confirmation_type.h"
#include "bat/ads/internal/ads_client_helper.h"
#include "bat/ads/internal/database/tables/transactions_database_table.h"
#include "bat/ads/internal/logging.h"
#include "bat/ads/pref_names.h"
#include "bat/ads/transaction_info.h"
#include "bat/ads/transaction_info_aliases.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

namespace ads {
namespace transactions {

namespace {

const char kFilename[] = "confirmations.json";

const char kTransactionHistoryListKey[] = "transaction_history";
const char kTransactionListKey[] = "transactions";
const char kCreatedAtKey[] = "timestamp_in_seconds";
const char kValueKey[] = "estimated_redemption_value";
const char kConfirmationTypeKey[] = "confirmation_type";

absl::optional<TransactionInfo> GetFromDictionary(
    const base::DictionaryValue* dictionary) {
  DCHECK(dictionary);

  // Created at
  const std::string* created_at_value =
      dictionary->FindStringKey(kCreatedAtKey);
  if (!created_at_value) {
    return absl::nullopt;
  }

  double created_at;
  if (!base::StringToDouble(*created_at_value, &created_at)) {
    return absl::nullopt;
  }

  // Value
  const std::string* value_value = dictionary->FindStringKey(kValueKey);
  if (!value_value) {
    return absl::nullopt;
  }

  double value;
  if (!base::StringToDouble(*value_value, &value)) {
    return absl::nullopt;
  }

  // Confirmation type
  const std::string* confirmation_type_value =
      dictionary->FindStringKey(kConfirmationTypeKey);
  if (!confirmation_type_value) {
    return absl::nullopt;
  }

  TransactionInfo transaction;
  transaction.created_at = created_at;
  transaction.value = value;
  transaction.confirmation_type = ConfirmationType(*confirmation_type_value);

  return transaction;
}

absl::optional<TransactionList> GetFromList(const base::Value* list) {
  DCHECK(list);
  DCHECK(list->is_list());

  TransactionList transactions;

  for (const auto& value : list->GetList()) {
    if (!value.is_dict()) {
      return absl::nullopt;
    }

    const base::DictionaryValue* dictionary = nullptr;
    value.GetAsDictionary(&dictionary);
    if (!dictionary) {
      return absl::nullopt;
    }

    const absl::optional<TransactionInfo> transaction_info =
        GetFromDictionary(dictionary);
    if (!transaction_info) {
      return absl::nullopt;
    }

    transactions.push_back(transaction.value());
  }

  return transactions;
}

absl::optional<TransactionList> FromJson(const std::string& json) {
  const absl::optional<base::Value> value = base::JSONReader::Read(json);
  if (!value || !value->is_dict()) {
    return absl::nullopt;
  }

  const base::DictionaryValue* dictionary = nullptr;
  if (!value->GetAsDictionary(&dictionary)) {
    return absl::nullopt;
  }

  const base::Value* transaction_history_list =
      dictionary->FindListKey(kTransactionHistoryListKey);
  if (!transaction_history_list) {
    return absl::nullopt;
  }

  const base::Value* list =
      transaction_history_list->FindListKey(kTransactionListKey);
  if (!list) {
    return absl::nullopt;
  }

  return GetFromList(list);
}

}  // namespace

void Migrate(InitializeCallback callback) {
  if (AdsClientHelper::Get()->GetBooleanPref(
          prefs::kHasMigratedTransactionState)) {
    callback(/* success */ true);
    return;
  }

  BLOG(3, "Loading transaction state");

  AdsClientHelper::Get()->Load(
      kFilename, [=](const bool success, const std::string& json) {
        if (!success) {
          // Transaction state does not exist
          BLOG(3, "Successfully migrated transaction state");

          AdsClientHelper::Get()->SetBooleanPref(
              prefs::kHasMigratedTransactionState, true);

          callback(/* success */ true);
          return;
        }

        const absl::optional<TransactionList> transactions = FromJson(json);

        if (!transactions) {
          BLOG(0, "Failed to migrate transactions");
          callback(/* success */ false);
          return;
        }

        BLOG(3, "Successfully loaded transaction state");

        BLOG(1, "Migrating transaction state");

        database::table::Transactions database_table;
        database_table.Save(
            transactions.value(), [=](const bool success) {
              if (!success) {
                BLOG(0, "Failed to migrate transaction state");
                callback(/* success */ false);
                return;
              }

              AdsClientHelper::Get()->SetBooleanPref(
                  prefs::kHasMigratedTransactionState, true);

              BLOG(3, "Successfully migrated transaction state");
              callback(/* success */ true);
            });
      });
}

}  // namespace transactions
}  // namespace ads
