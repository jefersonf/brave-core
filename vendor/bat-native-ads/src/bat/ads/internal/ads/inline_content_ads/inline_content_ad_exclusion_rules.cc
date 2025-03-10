/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/ads/inline_content_ads/inline_content_ad_exclusion_rules.h"

#include "base/check.h"
#include "bat/ads/internal/ad_serving/ad_targeting/geographic/subdivision/subdivision_targeting.h"
#include "bat/ads/internal/bundle/creative_ad_info.h"
#include "bat/ads/internal/frequency_capping/exclusion_rules/anti_targeting_frequency_cap.h"
#include "bat/ads/internal/frequency_capping/exclusion_rules/conversion_frequency_cap.h"
#include "bat/ads/internal/frequency_capping/exclusion_rules/daily_cap_frequency_cap.h"
#include "bat/ads/internal/frequency_capping/exclusion_rules/daypart_frequency_cap.h"
#include "bat/ads/internal/frequency_capping/exclusion_rules/dislike_frequency_cap.h"
#include "bat/ads/internal/frequency_capping/exclusion_rules/exclusion_rule.h"
#include "bat/ads/internal/frequency_capping/exclusion_rules/exclusion_rule_util.h"
#include "bat/ads/internal/frequency_capping/exclusion_rules/marked_as_inappropriate_frequency_cap.h"
#include "bat/ads/internal/frequency_capping/exclusion_rules/marked_to_no_longer_receive_frequency_cap.h"
#include "bat/ads/internal/frequency_capping/exclusion_rules/per_day_frequency_cap.h"
#include "bat/ads/internal/frequency_capping/exclusion_rules/per_hour_frequency_cap.h"
#include "bat/ads/internal/frequency_capping/exclusion_rules/per_month_frequency_cap.h"
#include "bat/ads/internal/frequency_capping/exclusion_rules/per_week_frequency_cap.h"
#include "bat/ads/internal/frequency_capping/exclusion_rules/split_test_frequency_cap.h"
#include "bat/ads/internal/frequency_capping/exclusion_rules/subdivision_targeting_frequency_cap.h"
#include "bat/ads/internal/frequency_capping/exclusion_rules/total_max_frequency_cap.h"
#include "bat/ads/internal/frequency_capping/exclusion_rules/transferred_frequency_cap.h"
#include "bat/ads/internal/resources/frequency_capping/anti_targeting_resource.h"

namespace ads {
namespace inline_content_ads {
namespace frequency_capping {

ExclusionRules::ExclusionRules(
    ad_targeting::geographic::SubdivisionTargeting* subdivision_targeting,
    resource::AntiTargeting* anti_targeting_resource,
    const AdEventList& ad_events,
    const BrowsingHistoryList& browsing_history)
    : subdivision_targeting_(subdivision_targeting),
      anti_targeting_resource_(anti_targeting_resource),
      ad_events_(ad_events),
      browsing_history_(browsing_history) {
  DCHECK(subdivision_targeting_);
  DCHECK(anti_targeting_resource_);
}

ExclusionRules::~ExclusionRules() = default;

bool ExclusionRules::ShouldExcludeCreativeAd(
    const CreativeAdInfo& creative_ad) const {
  bool should_exclude = false;

  DailyCapFrequencyCap daily_cap_frequency_cap(ad_events_);
  if (ShouldExclude(creative_ad, &daily_cap_frequency_cap)) {
    should_exclude = true;
  }

  PerDayFrequencyCap per_day_frequency_cap(ad_events_);
  if (ShouldExclude(creative_ad, &per_day_frequency_cap)) {
    should_exclude = true;
  }

  PerHourFrequencyCap per_hour_frequency_cap(ad_events_);
  if (ShouldExclude(creative_ad, &per_hour_frequency_cap)) {
    should_exclude = true;
  }

  PerWeekFrequencyCap per_week_frequency_cap(ad_events_);
  if (ShouldExclude(creative_ad, &per_week_frequency_cap)) {
    should_exclude = true;
  }

  PerMonthFrequencyCap per_month_frequency_cap(ad_events_);
  if (ShouldExclude(creative_ad, &per_month_frequency_cap)) {
    should_exclude = true;
  }

  TotalMaxFrequencyCap total_max_frequency_cap(ad_events_);
  if (ShouldExclude(creative_ad, &total_max_frequency_cap)) {
    should_exclude = true;
  }

  ConversionFrequencyCap conversion_frequency_cap(ad_events_);
  if (ShouldExclude(creative_ad, &conversion_frequency_cap)) {
    should_exclude = true;
  }

  SubdivisionTargetingFrequencyCap subdivision_frequency_cap(
      subdivision_targeting_);
  if (ShouldExclude(creative_ad, &subdivision_frequency_cap)) {
    should_exclude = true;
  }

  DaypartFrequencyCap daypart_frequency_cap;
  if (ShouldExclude(creative_ad, &daypart_frequency_cap)) {
    should_exclude = true;
  }

  TransferredFrequencyCap transferred_frequency_cap(ad_events_);
  if (ShouldExclude(creative_ad, &transferred_frequency_cap)) {
    should_exclude = true;
  }

  DislikeFrequencyCap dislike_frequency_cap;
  if (ShouldExclude(creative_ad, &dislike_frequency_cap)) {
    should_exclude = true;
  }

  MarkedToNoLongerReceiveFrequencyCap marked_to_no_longer_receive_frequency_cap;
  if (ShouldExclude(creative_ad, &marked_to_no_longer_receive_frequency_cap)) {
    should_exclude = true;
  }

  MarkedAsInappropriateFrequencyCap marked_as_inappropriate_frequency_cap;
  if (ShouldExclude(creative_ad, &marked_as_inappropriate_frequency_cap)) {
    should_exclude = true;
  }

  SplitTestFrequencyCap split_test_frequency_cap;
  if (ShouldExclude(creative_ad, &split_test_frequency_cap)) {
    should_exclude = true;
  }

  AntiTargetingFrequencyCap anti_targeting_frequency_cap(
      anti_targeting_resource_, browsing_history_);
  if (ShouldExclude(creative_ad, &anti_targeting_frequency_cap)) {
    should_exclude = true;
  }

  return should_exclude;
}

}  // namespace frequency_capping
}  // namespace inline_content_ads
}  // namespace ads
