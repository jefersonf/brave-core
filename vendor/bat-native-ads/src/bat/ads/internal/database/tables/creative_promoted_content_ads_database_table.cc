/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/database/tables/creative_promoted_content_ads_database_table.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "base/check.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/time/time.h"
#include "bat/ads/ads_client.h"
#include "bat/ads/internal/ads_client_helper.h"
#include "bat/ads/internal/bundle/creative_promoted_content_ad_info.h"
#include "bat/ads/internal/container_util.h"
#include "bat/ads/internal/database/database_statement_util.h"
#include "bat/ads/internal/database/database_table_util.h"
#include "bat/ads/internal/database/database_util.h"
#include "bat/ads/internal/database/tables/campaigns_database_table.h"
#include "bat/ads/internal/database/tables/creative_ads_database_table.h"
#include "bat/ads/internal/database/tables/dayparts_database_table.h"
#include "bat/ads/internal/database/tables/geo_targets_database_table.h"
#include "bat/ads/internal/database/tables/segments_database_table.h"
#include "bat/ads/internal/logging.h"
#include "bat/ads/internal/time_formatting_util.h"

namespace ads {
namespace database {
namespace table {

namespace {

const char kTableName[] = "creative_promoted_content_ads";

const int kDefaultBatchSize = 50;

}  // namespace

CreativePromotedContentAds::CreativePromotedContentAds()
    : batch_size_(kDefaultBatchSize),
      campaigns_database_table_(std::make_unique<Campaigns>()),
      creative_ads_database_table_(std::make_unique<CreativeAds>()),
      dayparts_database_table_(std::make_unique<Dayparts>()),
      geo_targets_database_table_(std::make_unique<GeoTargets>()),
      segments_database_table_(std::make_unique<Segments>()) {}

CreativePromotedContentAds::~CreativePromotedContentAds() = default;

void CreativePromotedContentAds::Save(
    const CreativePromotedContentAdList& creative_promoted_content_ads,
    ResultCallback callback) {
  if (creative_promoted_content_ads.empty()) {
    callback(/* success */ true);
    return;
  }

  mojom::DBTransactionPtr transaction = mojom::DBTransaction::New();

  const std::vector<CreativePromotedContentAdList> batches =
      SplitVector(creative_promoted_content_ads, batch_size_);

  for (const auto& batch : batches) {
    InsertOrUpdate(transaction.get(), batch);

    std::vector<CreativeAdInfo> creative_ads(batch.begin(), batch.end());
    campaigns_database_table_->InsertOrUpdate(transaction.get(), creative_ads);
    creative_ads_database_table_->InsertOrUpdate(transaction.get(),
                                                 creative_ads);
    dayparts_database_table_->InsertOrUpdate(transaction.get(), creative_ads);
    geo_targets_database_table_->InsertOrUpdate(transaction.get(),
                                                creative_ads);
    segments_database_table_->InsertOrUpdate(transaction.get(), creative_ads);
  }

  AdsClientHelper::Get()->RunDBTransaction(
      std::move(transaction),
      std::bind(&OnResultCallback, std::placeholders::_1, callback));
}

void CreativePromotedContentAds::Delete(ResultCallback callback) {
  mojom::DBTransactionPtr transaction = mojom::DBTransaction::New();

  util::Delete(transaction.get(), GetTableName());

  AdsClientHelper::Get()->RunDBTransaction(
      std::move(transaction),
      std::bind(&OnResultCallback, std::placeholders::_1, callback));
}

void CreativePromotedContentAds::GetForCreativeInstanceId(
    const std::string& creative_instance_id,
    GetCreativePromotedContentAdCallback callback) {
  CreativePromotedContentAdInfo creative_promoted_content_ad;

  if (creative_instance_id.empty()) {
    callback(/* success */ false, creative_instance_id,
             creative_promoted_content_ad);
    return;
  }

  const std::string query = base::StringPrintf(
      "SELECT "
      "cpca.creative_instance_id, "
      "cpca.creative_set_id, "
      "cpca.campaign_id, "
      "cam.start_at_timestamp, "
      "cam.end_at_timestamp, "
      "cam.daily_cap, "
      "cam.advertiser_id, "
      "cam.priority, "
      "ca.conversion, "
      "ca.per_day, "
      "ca.per_week, "
      "ca.per_month, "
      "ca.total_max, "
      "ca.value, "
      "s.segment, "
      "gt.geo_target, "
      "ca.target_url, "
      "cpca.title, "
      "cpca.description, "
      "cam.ptr, "
      "dp.dow, "
      "dp.start_minute, "
      "dp.end_minute "
      "FROM %s AS cpca "
      "INNER JOIN campaigns AS cam "
      "ON cam.campaign_id = cpca.campaign_id "
      "INNER JOIN segments AS s "
      "ON s.creative_set_id = cpca.creative_set_id "
      "INNER JOIN creative_ads AS ca "
      "ON ca.creative_instance_id = cpca.creative_instance_id "
      "INNER JOIN geo_targets AS gt "
      "ON gt.campaign_id = cpca.campaign_id "
      "INNER JOIN dayparts AS dp "
      "ON dp.campaign_id = cpca.campaign_id "
      "WHERE cpca.creative_instance_id = '%s'",
      GetTableName().c_str(), creative_instance_id.c_str());

  mojom::DBCommandPtr command = mojom::DBCommand::New();
  command->type = mojom::DBCommand::Type::READ;
  command->command = query;

  command->record_bindings = {
      mojom::DBCommand::RecordBindingType::STRING_TYPE,  // creative_instance_id
      mojom::DBCommand::RecordBindingType::STRING_TYPE,  // creative_set_id
      mojom::DBCommand::RecordBindingType::STRING_TYPE,  // campaign_id
      mojom::DBCommand::RecordBindingType::DOUBLE_TYPE,  // start_at
      mojom::DBCommand::RecordBindingType::DOUBLE_TYPE,  // end_at
      mojom::DBCommand::RecordBindingType::INT_TYPE,     // daily_cap
      mojom::DBCommand::RecordBindingType::STRING_TYPE,  // advertiser_id
      mojom::DBCommand::RecordBindingType::INT_TYPE,     // priority
      mojom::DBCommand::RecordBindingType::BOOL_TYPE,    // conversion
      mojom::DBCommand::RecordBindingType::INT_TYPE,     // per_day
      mojom::DBCommand::RecordBindingType::INT_TYPE,     // per_week
      mojom::DBCommand::RecordBindingType::INT_TYPE,     // per_month
      mojom::DBCommand::RecordBindingType::INT_TYPE,     // total_max
      mojom::DBCommand::RecordBindingType::DOUBLE_TYPE,  // value
      mojom::DBCommand::RecordBindingType::STRING_TYPE,  // segment
      mojom::DBCommand::RecordBindingType::STRING_TYPE,  // geo_target
      mojom::DBCommand::RecordBindingType::STRING_TYPE,  // target_url
      mojom::DBCommand::RecordBindingType::STRING_TYPE,  // title
      mojom::DBCommand::RecordBindingType::STRING_TYPE,  // description
      mojom::DBCommand::RecordBindingType::DOUBLE_TYPE,  // ptr
      mojom::DBCommand::RecordBindingType::STRING_TYPE,  // dayparts->dow
      mojom::DBCommand::RecordBindingType::INT_TYPE,  // dayparts->start_minute
      mojom::DBCommand::RecordBindingType::INT_TYPE   // dayparts->end_minute
  };

  mojom::DBTransactionPtr transaction = mojom::DBTransaction::New();
  transaction->commands.push_back(std::move(command));

  AdsClientHelper::Get()->RunDBTransaction(
      std::move(transaction),
      std::bind(&CreativePromotedContentAds::OnGetForCreativeInstanceId, this,
                std::placeholders::_1, creative_instance_id, callback));
}

void CreativePromotedContentAds::GetForSegments(
    const SegmentList& segments,
    GetCreativePromotedContentAdsCallback callback) {
  if (segments.empty()) {
    callback(/* success */ true, segments, {});
    return;
  }

  const std::string query = base::StringPrintf(
      "SELECT "
      "cpca.creative_instance_id, "
      "cpca.creative_set_id, "
      "cpca.campaign_id, "
      "cam.start_at_timestamp, "
      "cam.end_at_timestamp, "
      "cam.daily_cap, "
      "cam.advertiser_id, "
      "cam.priority, "
      "ca.conversion, "
      "ca.per_day, "
      "ca.per_week, "
      "ca.per_month, "
      "ca.total_max, "
      "ca.value, "
      "s.segment, "
      "gt.geo_target, "
      "ca.target_url, "
      "cpca.title, "
      "cpca.description, "
      "cam.ptr, "
      "dp.dow, "
      "dp.start_minute, "
      "dp.end_minute "
      "FROM %s AS cpca "
      "INNER JOIN campaigns AS cam "
      "ON cam.campaign_id = cpca.campaign_id "
      "INNER JOIN segments AS s "
      "ON s.creative_set_id = cpca.creative_set_id "
      "INNER JOIN creative_ads AS ca "
      "ON ca.creative_instance_id = cpca.creative_instance_id "
      "INNER JOIN geo_targets AS gt "
      "ON gt.campaign_id = cpca.campaign_id "
      "INNER JOIN dayparts AS dp "
      "ON dp.campaign_id = cpca.campaign_id "
      "WHERE s.segment IN %s "
      "AND %s BETWEEN cam.start_at_timestamp AND cam.end_at_timestamp",
      GetTableName().c_str(),
      BuildBindingParameterPlaceholder(segments.size()).c_str(),
      TimeAsTimestampString(base::Time::Now()).c_str());

  mojom::DBCommandPtr command = mojom::DBCommand::New();
  command->type = mojom::DBCommand::Type::READ;
  command->command = query;

  int index = 0;
  for (const auto& segment : segments) {
    BindString(command.get(), index, base::ToLowerASCII(segment));
    index++;
  }

  command->record_bindings = {
      mojom::DBCommand::RecordBindingType::STRING_TYPE,  // creative_instance_id
      mojom::DBCommand::RecordBindingType::STRING_TYPE,  // creative_set_id
      mojom::DBCommand::RecordBindingType::STRING_TYPE,  // campaign_id
      mojom::DBCommand::RecordBindingType::DOUBLE_TYPE,  // start_at
      mojom::DBCommand::RecordBindingType::DOUBLE_TYPE,  // end_at
      mojom::DBCommand::RecordBindingType::INT_TYPE,     // daily_cap
      mojom::DBCommand::RecordBindingType::STRING_TYPE,  // advertiser_id
      mojom::DBCommand::RecordBindingType::INT_TYPE,     // priority
      mojom::DBCommand::RecordBindingType::BOOL_TYPE,    // conversion
      mojom::DBCommand::RecordBindingType::INT_TYPE,     // per_day
      mojom::DBCommand::RecordBindingType::INT_TYPE,     // per_week
      mojom::DBCommand::RecordBindingType::INT_TYPE,     // per_month
      mojom::DBCommand::RecordBindingType::INT_TYPE,     // total_max
      mojom::DBCommand::RecordBindingType::DOUBLE_TYPE,  // value
      mojom::DBCommand::RecordBindingType::STRING_TYPE,  // segment
      mojom::DBCommand::RecordBindingType::STRING_TYPE,  // geo_target
      mojom::DBCommand::RecordBindingType::STRING_TYPE,  // target_url
      mojom::DBCommand::RecordBindingType::STRING_TYPE,  // title
      mojom::DBCommand::RecordBindingType::STRING_TYPE,  // description
      mojom::DBCommand::RecordBindingType::DOUBLE_TYPE,  // ptr
      mojom::DBCommand::RecordBindingType::STRING_TYPE,  // dayparts->dow
      mojom::DBCommand::RecordBindingType::INT_TYPE,  // dayparts->start_minute
      mojom::DBCommand::RecordBindingType::INT_TYPE   // dayparts->end_minute
  };

  mojom::DBTransactionPtr transaction = mojom::DBTransaction::New();
  transaction->commands.push_back(std::move(command));

  AdsClientHelper::Get()->RunDBTransaction(
      std::move(transaction),
      std::bind(&CreativePromotedContentAds::OnGetForSegments, this,
                std::placeholders::_1, segments, callback));
}

void CreativePromotedContentAds::GetAll(
    GetCreativePromotedContentAdsCallback callback) {
  const std::string query = base::StringPrintf(
      "SELECT "
      "cpca.creative_instance_id, "
      "cpca.creative_set_id, "
      "cpca.campaign_id, "
      "cam.start_at_timestamp, "
      "cam.end_at_timestamp, "
      "cam.daily_cap, "
      "cam.advertiser_id, "
      "cam.priority, "
      "ca.conversion, "
      "ca.per_day, "
      "ca.per_week, "
      "ca.per_month, "
      "ca.total_max, "
      "ca.value, "
      "s.segment, "
      "gt.geo_target, "
      "ca.target_url, "
      "cpca.title, "
      "cpca.description, "
      "cam.ptr, "
      "dp.dow, "
      "dp.start_minute, "
      "dp.end_minute "
      "FROM %s AS cpca "
      "INNER JOIN campaigns AS cam "
      "ON cam.campaign_id = cpca.campaign_id "
      "INNER JOIN segments AS s "
      "ON s.creative_set_id = cpca.creative_set_id "
      "INNER JOIN creative_ads AS ca "
      "ON ca.creative_instance_id = cpca.creative_instance_id "
      "INNER JOIN geo_targets AS gt "
      "ON gt.campaign_id = cpca.campaign_id "
      "INNER JOIN dayparts AS dp "
      "ON dp.campaign_id = cpca.campaign_id "
      "WHERE %s BETWEEN cam.start_at_timestamp AND cam.end_at_timestamp",
      GetTableName().c_str(), TimeAsTimestampString(base::Time::Now()).c_str());

  mojom::DBCommandPtr command = mojom::DBCommand::New();
  command->type = mojom::DBCommand::Type::READ;
  command->command = query;

  command->record_bindings = {
      mojom::DBCommand::RecordBindingType::STRING_TYPE,  // creative_instance_id
      mojom::DBCommand::RecordBindingType::STRING_TYPE,  // creative_set_id
      mojom::DBCommand::RecordBindingType::STRING_TYPE,  // campaign_id
      mojom::DBCommand::RecordBindingType::DOUBLE_TYPE,  // start_at
      mojom::DBCommand::RecordBindingType::DOUBLE_TYPE,  // end_at
      mojom::DBCommand::RecordBindingType::INT_TYPE,     // daily_cap
      mojom::DBCommand::RecordBindingType::STRING_TYPE,  // advertiser_id
      mojom::DBCommand::RecordBindingType::INT_TYPE,     // priority
      mojom::DBCommand::RecordBindingType::BOOL_TYPE,    // conversion
      mojom::DBCommand::RecordBindingType::INT_TYPE,     // per_day
      mojom::DBCommand::RecordBindingType::INT_TYPE,     // per_week
      mojom::DBCommand::RecordBindingType::INT_TYPE,     // per_month
      mojom::DBCommand::RecordBindingType::INT_TYPE,     // total_max
      mojom::DBCommand::RecordBindingType::DOUBLE_TYPE,  // value
      mojom::DBCommand::RecordBindingType::STRING_TYPE,  // segment
      mojom::DBCommand::RecordBindingType::STRING_TYPE,  // geo_target
      mojom::DBCommand::RecordBindingType::STRING_TYPE,  // target_url
      mojom::DBCommand::RecordBindingType::STRING_TYPE,  // title
      mojom::DBCommand::RecordBindingType::STRING_TYPE,  // description
      mojom::DBCommand::RecordBindingType::DOUBLE_TYPE,  // ptr
      mojom::DBCommand::RecordBindingType::STRING_TYPE,  // dayparts->dow
      mojom::DBCommand::RecordBindingType::INT_TYPE,  // dayparts->start_minute
      mojom::DBCommand::RecordBindingType::INT_TYPE   // dayparts->end_minute
  };

  mojom::DBTransactionPtr transaction = mojom::DBTransaction::New();
  transaction->commands.push_back(std::move(command));

  AdsClientHelper::Get()->RunDBTransaction(
      std::move(transaction), std::bind(&CreativePromotedContentAds::OnGetAll,
                                        this, std::placeholders::_1, callback));
}

std::string CreativePromotedContentAds::GetTableName() const {
  return kTableName;
}

void CreativePromotedContentAds::Migrate(mojom::DBTransaction* transaction,
                                         const int to_version) {
  DCHECK(transaction);

  switch (to_version) {
    case 16: {
      MigrateToV16(transaction);
      break;
    }

    default: {
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

void CreativePromotedContentAds::InsertOrUpdate(
    mojom::DBTransaction* transaction,
    const CreativePromotedContentAdList& creative_promoted_content_ads) {
  DCHECK(transaction);

  if (creative_promoted_content_ads.empty()) {
    return;
  }

  mojom::DBCommandPtr command = mojom::DBCommand::New();
  command->type = mojom::DBCommand::Type::RUN;
  command->command =
      BuildInsertOrUpdateQuery(command.get(), creative_promoted_content_ads);

  transaction->commands.push_back(std::move(command));
}

int CreativePromotedContentAds::BindParameters(
    mojom::DBCommand* command,
    const CreativePromotedContentAdList& creative_promoted_content_ads) {
  DCHECK(command);

  int count = 0;

  int index = 0;
  for (const auto& creative_promoted_content_ad :
       creative_promoted_content_ads) {
    BindString(command, index++,
               creative_promoted_content_ad.creative_instance_id);
    BindString(command, index++, creative_promoted_content_ad.creative_set_id);
    BindString(command, index++, creative_promoted_content_ad.campaign_id);
    BindString(command, index++, creative_promoted_content_ad.title);
    BindString(command, index++, creative_promoted_content_ad.description);

    count++;
  }

  return count;
}

std::string CreativePromotedContentAds::BuildInsertOrUpdateQuery(
    mojom::DBCommand* command,
    const CreativePromotedContentAdList& creative_promoted_content_ads) {
  const int count = BindParameters(command, creative_promoted_content_ads);

  return base::StringPrintf(
      "INSERT OR REPLACE INTO %s "
      "(creative_instance_id, "
      "creative_set_id, "
      "campaign_id, "
      "title, "
      "description) VALUES %s",
      GetTableName().c_str(),
      BuildBindingParameterPlaceholders(5, count).c_str());
}

void CreativePromotedContentAds::OnGetForCreativeInstanceId(
    mojom::DBCommandResponsePtr response,
    const std::string& creative_instance_id,
    GetCreativePromotedContentAdCallback callback) {
  if (!response ||
      response->status != mojom::DBCommandResponse::Status::RESPONSE_OK) {
    BLOG(0, "Failed to get creative new tab page ad");
    callback(/* success */ false, creative_instance_id, {});
    return;
  }

  if (response->result->get_records().size() != 1) {
    BLOG(0, "Failed to get creative new tab page ad");
    callback(/* success */ false, creative_instance_id, {});
    return;
  }

  mojom::DBRecord* record = response->result->get_records().at(0).get();

  const CreativePromotedContentAdInfo creative_promoted_content_ad =
      GetFromRecord(record);

  callback(/* success */ true, creative_instance_id,
           creative_promoted_content_ad);
}

void CreativePromotedContentAds::OnGetForSegments(
    mojom::DBCommandResponsePtr response,
    const SegmentList& segments,
    GetCreativePromotedContentAdsCallback callback) {
  if (!response ||
      response->status != mojom::DBCommandResponse::Status::RESPONSE_OK) {
    BLOG(0, "Failed to get creative new tab page ads");
    callback(/* success */ false, segments, {});
    return;
  }

  CreativePromotedContentAdList creative_promoted_content_ads;

  for (const auto& record : response->result->get_records()) {
    const CreativePromotedContentAdInfo creative_promoted_content_ad =
        GetFromRecord(record.get());

    creative_promoted_content_ads.push_back(creative_promoted_content_ad);
  }

  callback(/* success */ true, segments, creative_promoted_content_ads);
}

void CreativePromotedContentAds::OnGetAll(
    mojom::DBCommandResponsePtr response,
    GetCreativePromotedContentAdsCallback callback) {
  if (!response ||
      response->status != mojom::DBCommandResponse::Status::RESPONSE_OK) {
    BLOG(0, "Failed to get all creative new tab page ads");
    callback(/* success */ false, {}, {});
    return;
  }

  CreativePromotedContentAdList creative_promoted_content_ads;

  SegmentList segments;

  for (const auto& record : response->result->get_records()) {
    const CreativePromotedContentAdInfo creative_promoted_content_ad =
        GetFromRecord(record.get());

    creative_promoted_content_ads.push_back(creative_promoted_content_ad);

    segments.push_back(creative_promoted_content_ad.segment);
  }

  std::sort(segments.begin(), segments.end());
  const auto iter = std::unique(segments.begin(), segments.end());
  segments.erase(iter, segments.end());

  callback(/* success */ true, segments, creative_promoted_content_ads);
}

CreativePromotedContentAdInfo CreativePromotedContentAds::GetFromRecord(
    mojom::DBRecord* record) const {
  CreativePromotedContentAdInfo creative_promoted_content_ad;

  creative_promoted_content_ad.creative_instance_id = ColumnString(record, 0);
  creative_promoted_content_ad.creative_set_id = ColumnString(record, 1);
  creative_promoted_content_ad.campaign_id = ColumnString(record, 2);
  creative_promoted_content_ad.start_at =
      base::Time::FromDoubleT(ColumnDouble(record, 3));
  creative_promoted_content_ad.end_at =
      base::Time::FromDoubleT(ColumnDouble(record, 4));
  creative_promoted_content_ad.daily_cap = ColumnInt(record, 5);
  creative_promoted_content_ad.advertiser_id = ColumnString(record, 6);
  creative_promoted_content_ad.priority = ColumnInt(record, 7);
  creative_promoted_content_ad.conversion = ColumnBool(record, 8);
  creative_promoted_content_ad.per_day = ColumnInt(record, 9);
  creative_promoted_content_ad.per_week = ColumnInt(record, 10);
  creative_promoted_content_ad.per_month = ColumnInt(record, 11);
  creative_promoted_content_ad.total_max = ColumnInt(record, 12);
  creative_promoted_content_ad.value = ColumnDouble(record, 13);
  creative_promoted_content_ad.segment = ColumnString(record, 14);
  creative_promoted_content_ad.geo_targets.push_back(ColumnString(record, 15));
  creative_promoted_content_ad.target_url = ColumnString(record, 16);
  creative_promoted_content_ad.title = ColumnString(record, 17);
  creative_promoted_content_ad.description = ColumnString(record, 18);
  creative_promoted_content_ad.ptr = ColumnDouble(record, 19);

  CreativeDaypartInfo daypart;
  daypart.dow = ColumnString(record, 20);
  daypart.start_minute = ColumnInt(record, 21);
  daypart.end_minute = ColumnInt(record, 22);
  creative_promoted_content_ad.dayparts.push_back(daypart);

  return creative_promoted_content_ad;
}

void CreativePromotedContentAds::CreateTableV16(
    mojom::DBTransaction* transaction) {
  DCHECK(transaction);

  const std::string query = base::StringPrintf(
      "CREATE TABLE %s "
      "(creative_instance_id TEXT NOT NULL PRIMARY KEY UNIQUE "
      "ON CONFLICT REPLACE, "
      "creative_set_id TEXT NOT NULL, "
      "campaign_id TEXT NOT NULL, "
      "title TEXT NOT NULL, "
      "description TEXT NOT NULL)",
      GetTableName().c_str());

  mojom::DBCommandPtr command = mojom::DBCommand::New();
  command->type = mojom::DBCommand::Type::EXECUTE;
  command->command = query;

  transaction->commands.push_back(std::move(command));
}

void CreativePromotedContentAds::MigrateToV16(
    mojom::DBTransaction* transaction) {
  DCHECK(transaction);

  util::Drop(transaction, GetTableName());

  CreateTableV16(transaction);
}

}  // namespace table
}  // namespace database
}  // namespace ads
