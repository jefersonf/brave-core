/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/account/rewards_util.h"

#include "bat/ads/confirmation_type.h"
#include "bat/ads/internal/account/confirmations/confirmations_state.h"

namespace ads {

void RewardUser(const double value, const ConfirmationType& confirmation_type) {
  // TODO(tmancey): Implement
}

void ResetRewards() {
  // TODO(tmancey): Delete all transactions from database table

  // TODO(tmancey): Decouple logic from here and move to
  // ConfirmationsState::Get->RemoveFailedConfirmations()
  ConfirmationsState::Get()->reset_failed_confirmations();

  // TODO(tmancey): Decouple logic from here and move to
  // ConfirmationsState::Get->RemoveAllUnblindedPaymentTokens()
  privacy::UnblindedTokens* unblinded_payment_tokens =
      ConfirmationsState::Get()->get_unblinded_payment_tokens();
  unblinded_payment_tokens->RemoveAllTokens();

  ConfirmationsState::Get()->Save();
}

}  // namespace ads
