/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ACCOUNT_STATEMENT_STATEMENT_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ACCOUNT_STATEMENT_STATEMENT_H_

#include "bat/ads/internal/account/statement/statement_aliases.h"

namespace base {
class Time;
}  // namespace base

namespace ads {

struct StatementInfo;

class Statement final {
 public:
  Statement();
  ~Statement();

  StatementInfo Get(StatementCallback callback) const;

 private:
  base::Time GetNextPaymentDate() const;
  double GetEarningsForThisMonth() const;
  double GetEarningsForLastMonth() const;
  int GetAdsReceivedForThisMonth() const;
};

}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ACCOUNT_STATEMENT_STATEMENT_H_
