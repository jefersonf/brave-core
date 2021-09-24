/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/account/transactions/transactions.h"

#include <string>

#include "base/check.h"
#include "base/notreached.h"
#include "base/time/time.h"
#include "bat/ads/internal/account/confirmations/confirmation_info.h"
#include "bat/ads/internal/account/confirmations/confirmations_state.h"
#include "bat/ads/internal/privacy/unblinded_tokens/unblinded_tokens.h"

namespace ads {
namespace transactions {

// TODO(tmancey): Refactor to return kViewed count for a list of transactions
uint64_t GetCountForMonth(const base::Time& time) {
  const TransactionList transactions =
      ConfirmationsState::Get()->GetTransactions();

  uint64_t count = 0;

  base::Time::Exploded exploded;
  time.LocalExplode(&exploded);

  for (const auto& transaction : transactions) {
    const base::Time transaction_time =
        base::Time::FromDoubleT(transaction.created_at);

    base::Time::Exploded transaction_time_exploded;
    transaction_time.LocalExplode(&transaction_time_exploded);

    if (transaction_time_exploded.year == exploded.year &&
        transaction_time_exploded.month == exploded.month &&
        ConfirmationType(transaction.confirmation_type) ==
            ConfirmationType::kViewed) {
      count++;
    }
  }

  return count;
}

// TODO(tmancey): Rename object to transactions util
void Add(const double value, const ConfirmationInfo& confirmation) {
  DCHECK(confirmation.IsValid());

  TransactionInfo transaction;

  transaction.created_at = base::Time::Now().ToDoubleT();
  transaction.value = value;
  // TODO(tmancey): Remove need to store as a string
  transaction.confirmation_type = std::string(confirmation.type);

  database::table::Transactions database_table;
  database_table.Save({transaction}, [](const bool success) {
    if (!success) {
      BLOG(0, "Failed to append transaction");
      return;
    }

    BLOG(3, "Successfully appended transaction");
  });
}

}  // namespace transactions
}  // namespace ads
