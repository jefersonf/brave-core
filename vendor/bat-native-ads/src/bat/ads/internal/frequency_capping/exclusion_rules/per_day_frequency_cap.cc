/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/frequency_capping/exclusion_rules/per_day_frequency_cap.h"

#include <deque>

#include "base/strings/stringprintf.h"
#include "base/time/time.h"
#include "bat/ads/internal/frequency_capping/frequency_capping_util.h"

namespace ads {

PerDayFrequencyCap::PerDayFrequencyCap(const AdEventList& ad_events)
    : ad_events_(ad_events) {}

PerDayFrequencyCap::~PerDayFrequencyCap() = default;

std::string PerDayFrequencyCap::GetUuid(
    const CreativeAdInfo& creative_ad) const {
  return creative_ad.creative_set_id;
}

bool PerDayFrequencyCap::ShouldExclude(const CreativeAdInfo& creative_ad) {
  const AdEventList filtered_ad_events =
      FilterAdEvents(ad_events_, creative_ad);

  if (!DoesRespectCap(filtered_ad_events, creative_ad)) {
    last_message_ = base::StringPrintf(
        "creativeSetId %s has exceeded the perDay frequency cap",
        creative_ad.creative_set_id.c_str());

    return true;
  }

  return false;
}

std::string PerDayFrequencyCap::GetLastMessage() const {
  return last_message_;
}

bool PerDayFrequencyCap::DoesRespectCap(const AdEventList& ad_events,
                                        const CreativeAdInfo& creative_ad) {
  if (creative_ad.per_day == 0) {
    return true;
  }

  const base::TimeDelta time_constraint = base::TimeDelta::FromSeconds(
      base::Time::kSecondsPerHour * base::Time::kHoursPerDay);

  return DoesAdEventsRespectCapForRollingTimeConstraint(
      ad_events, time_constraint, creative_ad.per_day);
}

AdEventList PerDayFrequencyCap::FilterAdEvents(
    const AdEventList& ad_events,
    const CreativeAdInfo& creative_ad) const {
  AdEventList filtered_ad_events = ad_events;

  const auto iter = std::remove_if(
      filtered_ad_events.begin(), filtered_ad_events.end(),
      [&creative_ad](const AdEventInfo& ad_event) {
        return !DoesAdTypeSupportFrequencyCapping(ad_event.type) ||
               ad_event.creative_set_id != creative_ad.creative_set_id ||
               ad_event.confirmation_type != ConfirmationType::kServed;
      });

  filtered_ad_events.erase(iter, filtered_ad_events.end());

  return filtered_ad_events;
}

}  // namespace ads
