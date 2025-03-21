/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/frequency_capping/exclusion_rules/subdivision_targeting_frequency_cap.h"

#include <vector>

#include "base/check.h"
#include "base/strings/string_split.h"
#include "base/strings/stringprintf.h"
#include "bat/ads/internal/ad_serving/ad_targeting/geographic/subdivision/subdivision_targeting.h"
#include "bat/ads/internal/locale/subdivision_code_util.h"
#include "brave/components/l10n/browser/locale_helper.h"

namespace ads {

namespace {

bool DoesAdSupportSubdivisionTargetingCode(
    const CreativeAdInfo& creative_ad,
    const std::string& subdivision_targeting_code) {
  const std::string country_code =
      locale::GetCountryCode(subdivision_targeting_code);

  const auto iter =
      std::find_if(creative_ad.geo_targets.begin(),
                   creative_ad.geo_targets.end(),
                   [&subdivision_targeting_code,
                    &country_code](const std::string& geo_target) {
                     return geo_target == subdivision_targeting_code ||
                            geo_target == country_code;
                   });

  if (iter == creative_ad.geo_targets.end()) {
    return false;
  }

  return true;
}

bool DoesAdTargetSubdivision(const CreativeAdInfo& creative_ad) {
  const auto iter = std::find_if(
      creative_ad.geo_targets.begin(), creative_ad.geo_targets.end(),
      [](const std::string& geo_target) {
        const std::vector<std::string> components = base::SplitString(
            geo_target, "-", base::KEEP_WHITESPACE, base::SPLIT_WANT_ALL);

        return components.size() == 2;
      });

  if (iter == creative_ad.geo_targets.end()) {
    return false;
  }

  return true;
}

}  // namespace

SubdivisionTargetingFrequencyCap::SubdivisionTargetingFrequencyCap(
    ad_targeting::geographic::SubdivisionTargeting* subdivision_targeting)
    : subdivision_targeting_(subdivision_targeting) {
  DCHECK(subdivision_targeting_);
}

SubdivisionTargetingFrequencyCap::~SubdivisionTargetingFrequencyCap() = default;

bool SubdivisionTargetingFrequencyCap::ShouldExclude(
    const CreativeAdInfo& creative_ad) {
  if (!DoesRespectCap(creative_ad)) {
    last_message_ = base::StringPrintf(
        "creativeSetId %s excluded as not within the targeted subdivision",
        creative_ad.creative_set_id.c_str());

    return true;
  }

  return false;
}

std::string SubdivisionTargetingFrequencyCap::GetLastMessage() const {
  return last_message_;
}

bool SubdivisionTargetingFrequencyCap::DoesRespectCap(
    const CreativeAdInfo& creative_ad) {
  const std::string locale =
      brave_l10n::LocaleHelper::GetInstance()->GetLocale();

  if (!subdivision_targeting_->ShouldAllowForLocale(locale)) {
    return !DoesAdTargetSubdivision(creative_ad);
  }

  if (subdivision_targeting_->IsDisabled()) {
    return !DoesAdTargetSubdivision(creative_ad);
  }

  const std::string subdivision_targeting_code =
      subdivision_targeting_->GetAdsSubdivisionTargetingCode();

  if (subdivision_targeting_code.empty()) {
    return false;
  }

  return DoesAdSupportSubdivisionTargetingCode(creative_ad,
                                               subdivision_targeting_code);
}

}  // namespace ads
