/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "base/guid.h"
#include "bat/ads/internal/ad_serving/ad_notifications/ad_notification_serving.h"
#include "bat/ads/internal/ad_serving/ad_targeting/geographic/subdivision/subdivision_targeting.h"
#include "bat/ads/internal/database/tables/creative_ad_notifications_database_table.h"
#include "bat/ads/internal/frequency_capping/frequency_capping_unittest_util.h"
#include "bat/ads/internal/resources/frequency_capping/anti_targeting_resource.h"
#include "bat/ads/internal/unittest_base.h"
#include "bat/ads/internal/unittest_time_util.h"
#include "bat/ads/internal/unittest_util.h"
#include "bat/ads/internal/user_activity/user_activity.h"
#include "net/http/http_status_code.h"

// npm run test -- brave_unit_tests --filter=BatAds*

using ::testing::_;
using ::testing::AllOf;
using ::testing::Between;
using ::testing::Field;
using ::testing::Matcher;

namespace ads {

namespace {

Matcher<const AdNotificationInfo&> DoesMatchCreativeInstanceId(
    const std::string& creative_instance_id) {
  return AllOf(Field("creative_instance_id",
                     &AdNotificationInfo::creative_instance_id,
                     creative_instance_id));
}

void ServeAd() {
  ad_targeting::geographic::SubdivisionTargeting subdivision_targeting;
  resource::AntiTargeting anti_targeting_resource;
  ad_notifications::AdServing ad_serving(&subdivision_targeting,
                                         &anti_targeting_resource);

  ad_serving.MaybeServeAd();
}

}  // namespace

class BatAdsAdPriorityTest : public UnitTestBase {
 protected:
  BatAdsAdPriorityTest()
      : database_table_(
            std::make_unique<database::table::CreativeAdNotifications>()) {}

  ~BatAdsAdPriorityTest() override = default;

  void SetUp() override {
    ASSERT_TRUE(CopyFileFromTestPathToTempDir(
        "confirmations_with_unblinded_tokens.json", "confirmations.json"));

    UnitTestBase::SetUpForTesting(/* integration_test */ true);

    const URLEndpoints endpoints = {
        {"/v9/catalog", {{net::HTTP_OK, "/empty_catalog.json"}}},
        {// Get issuers request
         R"(/v1/issuers/)",
         {{net::HTTP_OK, R"(
        {
          "ping": 7200000,
          "issuers": [
            {
              "name": "confirmations",
              "publicKeys": [
                "JsvJluEN35bJBgJWTdW/8dAgPrrTM1I1pXga+o7cllo=",
                "crDVI1R6xHQZ4D9cQu4muVM5MaaM1QcOT4It8Y/CYlw="
              ]
            },
            {
              "name": "payments",
              "publicKeys": [
                "JiwFR2EU/Adf1lgox+xqOVPuc6a/rxdy/LguFG5eaXg=",
                "XgxwreIbLMu0IIFVk4TKEce6RduNVXngDmU3uixly0M=",
                "bPE1QE65mkIgytffeu7STOfly+x10BXCGuk5pVlOHQU="
              ]
            }
          ]
        }
        )"}}}};
    MockUrlRequest(ads_client_mock_, endpoints);

    InitializeAds();

    RecordUserActivityEvents();
  }

  void RecordUserActivityEvents() {
    UserActivity::Get()->RecordEvent(UserActivityEventType::kOpenedNewTab);
    UserActivity::Get()->RecordEvent(UserActivityEventType::kClosedTab);
  }

  CreativeAdNotificationInfo GetCreativeAdNotification() {
    CreativeAdNotificationInfo creative_ad_notification;

    creative_ad_notification.creative_instance_id = base::GenerateGUID();
    creative_ad_notification.creative_set_id = base::GenerateGUID();
    creative_ad_notification.campaign_id = base::GenerateGUID();
    creative_ad_notification.start_at = DistantPast();
    creative_ad_notification.end_at = DistantFuture();
    creative_ad_notification.daily_cap = 1;
    creative_ad_notification.advertiser_id = base::GenerateGUID();
    creative_ad_notification.priority = 1;
    creative_ad_notification.ptr = 1.0;
    creative_ad_notification.per_day = 1;
    creative_ad_notification.per_week = 1;
    creative_ad_notification.per_month = 1;
    creative_ad_notification.total_max = 1;
    creative_ad_notification.value = 1.0;
    creative_ad_notification.segment = "untargeted";
    creative_ad_notification.geo_targets = {"US"};
    creative_ad_notification.target_url = "https://brave.com";
    CreativeDaypartInfo daypart;
    creative_ad_notification.dayparts = {daypart};
    creative_ad_notification.title = "Test Ad Title";
    creative_ad_notification.body = "Test Ad Body";

    return creative_ad_notification;
  }

  void ServeAdForIterations(const int iterations) {
    for (int i = 0; i < iterations; i++) {
      ResetFrequencyCaps(AdType::kAdNotification);

      ad_targeting::geographic::SubdivisionTargeting subdivision_targeting;
      resource::AntiTargeting anti_targeting_resource;
      ad_notifications::AdServing ad_serving(&subdivision_targeting,
                                             &anti_targeting_resource);

      ad_serving.MaybeServeAd();
    }
  }

  void Save(const CreativeAdNotificationList& creative_ad_notifications) {
    database_table_->Save(creative_ad_notifications,
                          [](const bool success) { ASSERT_TRUE(success); });
  }

  std::unique_ptr<database::table::CreativeAdNotifications> database_table_;
};

TEST_F(BatAdsAdPriorityTest, PrioritizeDeliveryForSingleAd) {
  // Arrange
  CreativeAdNotificationList creative_ad_notifications;

  CreativeAdNotificationInfo creative_ad_notification =
      GetCreativeAdNotification();
  creative_ad_notification.priority = 3;
  creative_ad_notifications.push_back(creative_ad_notification);

  Save(creative_ad_notifications);

  // Act
  EXPECT_CALL(*ads_client_mock_,
              ShowNotification(DoesMatchCreativeInstanceId(
                  creative_ad_notification.creative_instance_id)))
      .Times(1);

  ServeAd();

  // Assert
}

TEST_F(BatAdsAdPriorityTest, PrioritizeDeliveryForNoAds) {
  // Arrange

  // Act
  EXPECT_CALL(*ads_client_mock_, ShowNotification(_)).Times(0);

  ServeAd();

  // Assert
}

TEST_F(BatAdsAdPriorityTest, PrioritizeDeliveryForMultipleAds) {
  // Arrange
  CreativeAdNotificationList creative_ad_notifications;

  CreativeAdNotificationInfo creative_ad_notification_1 =
      GetCreativeAdNotification();
  creative_ad_notification_1.priority = 3;
  creative_ad_notifications.push_back(creative_ad_notification_1);

  CreativeAdNotificationInfo creative_ad_notification_2 =
      GetCreativeAdNotification();
  creative_ad_notification_2.priority = 2;
  creative_ad_notifications.push_back(creative_ad_notification_2);

  CreativeAdNotificationInfo creative_ad_notification_3 =
      GetCreativeAdNotification();
  creative_ad_notification_3.priority = 4;
  creative_ad_notifications.push_back(creative_ad_notification_3);

  Save(creative_ad_notifications);

  // Act
  EXPECT_CALL(*ads_client_mock_,
              ShowNotification(DoesMatchCreativeInstanceId(
                  creative_ad_notification_2.creative_instance_id)))
      .Times(1);

  ServeAd();

  // Assert
}

}  // namespace ads
