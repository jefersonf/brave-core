/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/frequency_capping/exclusion_rules/daily_cap_frequency_cap.h"

#include <deque>

#include "base/strings/stringprintf.h"
#include "base/time/time.h"
#include "bat/ads/internal/frequency_capping/frequency_capping_util.h"

namespace ads {

DailyCapFrequencyCap::DailyCapFrequencyCap(const AdEventList& ad_events)
    : ad_events_(ad_events) {}

DailyCapFrequencyCap::~DailyCapFrequencyCap() = default;

bool DailyCapFrequencyCap::ShouldExclude(const CreativeAdInfo& creative_ad) {
  const AdEventList filtered_ad_events =
      FilterAdEvents(ad_events_, creative_ad);

  if (!DoesRespectCap(filtered_ad_events, creative_ad)) {
    last_message_ = base::StringPrintf(
        "campaignId %s has exceeded the frequency capping for dailyCap",
        creative_ad.campaign_id.c_str());

    return true;
  }

  return false;
}

std::string DailyCapFrequencyCap::GetLastMessage() const {
  return last_message_;
}

bool DailyCapFrequencyCap::DoesRespectCap(const AdEventList& ad_events,
                                          const CreativeAdInfo& creative_ad) {
  const std::deque<base::Time> history = GetHistoryForAdEvents(ad_events);

  const base::TimeDelta time_constraint = base::TimeDelta::FromSeconds(
      base::Time::kSecondsPerHour * base::Time::kHoursPerDay);

  return DoesHistoryRespectCapForRollingTimeConstraint(history, time_constraint,
                                                       creative_ad.daily_cap);
}

AdEventList DailyCapFrequencyCap::FilterAdEvents(
    const AdEventList& ad_events,
    const CreativeAdInfo& creative_ad) const {
  AdEventList filtered_ad_events = ad_events;

  const auto iter = std::remove_if(
      filtered_ad_events.begin(), filtered_ad_events.end(),
      [&creative_ad](const AdEventInfo& ad_event) {
        return (ad_event.type != AdType::kAdNotification &&
                ad_event.type != AdType::kInlineContentAd) ||
               ad_event.campaign_id != creative_ad.campaign_id ||
               ad_event.confirmation_type != ConfirmationType::kServed;
      });

  filtered_ad_events.erase(iter, filtered_ad_events.end());

  return filtered_ad_events;
}

}  // namespace ads
