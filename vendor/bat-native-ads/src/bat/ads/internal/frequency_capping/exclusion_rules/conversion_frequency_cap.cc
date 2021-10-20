/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/frequency_capping/exclusion_rules/conversion_frequency_cap.h"

#include <cstdint>

#include "base/strings/stringprintf.h"
#include "bat/ads/ads_client.h"
#include "bat/ads/internal/ads_client_helper.h"
#include "bat/ads/internal/frequency_capping/frequency_capping_features.h"
#include "bat/ads/internal/frequency_capping/frequency_capping_util.h"
#include "bat/ads/pref_names.h"

namespace ads {

namespace {
const uint64_t kConversionFrequencyCap = 1;
}  // namespace

ConversionFrequencyCap::ConversionFrequencyCap(const AdEventList& ad_events)
    : ad_events_(ad_events) {}

ConversionFrequencyCap::~ConversionFrequencyCap() = default;

std::string ConversionFrequencyCap::GetUuid(
    const CreativeAdInfo& creative_ad) const {
  return creative_ad.creative_set_id;
}

bool ConversionFrequencyCap::ShouldExclude(const CreativeAdInfo& creative_ad) {
  if (!features::frequency_capping::ShouldExcludeAdIfConverted()) {
    return false;
  }

  if (!ShouldAllow(creative_ad)) {
    last_message_ = base::StringPrintf(
        "creativeSetId %s excluded due to disabled ad conversion tracking",
        creative_ad.creative_set_id.c_str());

    return true;
  }

  const AdEventList filtered_ad_events =
      FilterAdEvents(ad_events_, creative_ad);

  if (!DoesRespectCap(filtered_ad_events)) {
    last_message_ = base::StringPrintf(
        "creativeSetId %s has exceeded the conversions frequency cap",
        creative_ad.creative_set_id.c_str());

    return true;
  }

  return false;
}

std::string ConversionFrequencyCap::GetLastMessage() const {
  return last_message_;
}

bool ConversionFrequencyCap::ShouldAllow(const CreativeAdInfo& creative_ad) {
  if (creative_ad.conversion && !AdsClientHelper::Get()->GetBooleanPref(
                                    prefs::kShouldAllowConversionTracking)) {
    return false;
  }

  return true;
}

bool ConversionFrequencyCap::DoesRespectCap(const AdEventList& ad_events) {
  if (ad_events.size() >= kConversionFrequencyCap) {
    return false;
  }

  return true;
}

AdEventList ConversionFrequencyCap::FilterAdEvents(
    const AdEventList& ad_events,
    const CreativeAdInfo& creative_ad) const {
  AdEventList filtered_ad_events = ad_events;

  const auto iter = std::remove_if(
      filtered_ad_events.begin(), filtered_ad_events.end(),
      [&creative_ad](const AdEventInfo& ad_event) {
        return !DoesAdTypeSupportFrequencyCapping(ad_event.type) ||
               ad_event.creative_set_id != creative_ad.creative_set_id ||
               ad_event.confirmation_type != ConfirmationType::kConversion;
      });

  filtered_ad_events.erase(iter, filtered_ad_events.end());

  return filtered_ad_events;
}

}  // namespace ads
