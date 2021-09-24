/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ACCOUNT_AD_REWARDS_AD_REWARDS_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ACCOUNT_AD_REWARDS_AD_REWARDS_H_

#include <cstdint>

namespace base {
class Time;
}  // namespace base

namespace ads {

class AdRewards final {
 public:
  AdRewards();
  ~AdRewards();

  double GetNextPaymentDate() const;

  int GetAdsReceivedForThisMonth() const;
  int GetAdsReceivedForMonth(const base::Time& time) const;

  double GetEarningsForThisMonth() const;
  double GetEarningsForMonth(const base::Time& time) const;

  void Reset();
};

}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ACCOUNT_AD_REWARDS_AD_REWARDS_H_
