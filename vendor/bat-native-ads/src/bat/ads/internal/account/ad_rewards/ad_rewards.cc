/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/account/ad_rewards/ad_rewards.h"

#include "base/time/time.h"
#include "bat/ads/internal/account/confirmations/confirmations_state.h"
#include "bat/ads/internal/account/transactions/transactions.h"
#include "bat/ads/internal/privacy/unblinded_tokens/unblinded_tokens.h"

namespace ads {

AdRewards::AdRewards() : payments_(std::make_unique<Payments>()) {}

AdRewards::~AdRewards() {
  delegate_ = nullptr;
}

double AdRewards::GetNextPaymentDate() const {
  const base::Time now = base::Time::Now();

  const base::Time next_token_redemption_date =
      ConfirmationsState::Get()->GetNextTokenRedemptionDate();

  const base::Time next_payment_date =
      payments_->CalculateNextPaymentDate(now, next_token_redemption_date);

  return next_payment_date.ToDoubleT();
}

int AdRewards::GetAdsReceivedForThisMonth() const {
  const base::Time now = base::Time::Now();
  return GetAdsReceivedForMonth(now);
}

int AdRewards::GetAdsReceivedForMonth(const base::Time& time) const {
  // TODO(tmancey): Get transactions from database table
  const TransactionList transactions =
      ConfirmationsState::Get()->GetTransactions();

  int ads_received_this_month = 0;

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
      ads_received_this_month++;
    }
  }

  return ads_received_this_month;
}

double AdRewards::GetEarningsForThisMonth() const {
  const base::Time now = base::Time::Now();
  return GetEarningsForMonth(now);
}

double AdRewards::GetEarningsForMonth(const base::Time& time) const {
  const PaymentInfo payment = payments_->GetForMonth(time);
  return payment.balance;
}

void AdRewards::Reset() {
  payments_->reset();

  ConfirmationsState::Get()->reset_failed_confirmations();

  // TODO(tmancey): Delete all transactions from database table
  ConfirmationsState::Get()->reset_transactions();

  privacy::UnblindedTokens* unblinded_payment_tokens =
      ConfirmationsState::Get()->get_unblinded_payment_tokens();
  unblinded_payment_tokens->RemoveAllTokens();

  ConfirmationsState::Get()->Save();
}

}  // namespace ads
