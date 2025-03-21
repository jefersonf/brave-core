/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/database/tables/creative_ads_database_table.h"

#include <algorithm>
#include <utility>

#include "base/check.h"
#include "base/strings/stringprintf.h"
#include "bat/ads/ads_client.h"
#include "bat/ads/internal/ads_client_helper.h"
#include "bat/ads/internal/database/database_statement_util.h"
#include "bat/ads/internal/database/database_table_util.h"
#include "bat/ads/internal/database/database_util.h"
#include "bat/ads/internal/logging.h"

namespace ads {
namespace database {
namespace table {

namespace {
const char kTableName[] = "creative_ads";
}  // namespace

CreativeAds::CreativeAds() = default;

CreativeAds::~CreativeAds() = default;

void CreativeAds::InsertOrUpdate(mojom::DBTransaction* transaction,
                                 const CreativeAdList& creative_ads) {
  DCHECK(transaction);

  if (creative_ads.empty()) {
    return;
  }

  mojom::DBCommandPtr command = mojom::DBCommand::New();
  command->type = mojom::DBCommand::Type::RUN;
  command->command = BuildInsertOrUpdateQuery(command.get(), creative_ads);

  transaction->commands.push_back(std::move(command));
}

void CreativeAds::Delete(ResultCallback callback) {
  mojom::DBTransactionPtr transaction = mojom::DBTransaction::New();

  util::Delete(transaction.get(), GetTableName());

  AdsClientHelper::Get()->RunDBTransaction(
      std::move(transaction),
      std::bind(&OnResultCallback, std::placeholders::_1, callback));
}

std::string CreativeAds::GetTableName() const {
  return kTableName;
}

void CreativeAds::Migrate(mojom::DBTransaction* transaction,
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

int CreativeAds::BindParameters(mojom::DBCommand* command,
                                const CreativeAdList& creative_ads) {
  DCHECK(command);

  int count = 0;

  int index = 0;
  for (const auto& creative_ad : creative_ads) {
    BindString(command, index++, creative_ad.creative_instance_id);
    BindBool(command, index++, creative_ad.conversion);
    BindInt(command, index++, creative_ad.per_day);
    BindInt(command, index++, creative_ad.per_week);
    BindInt(command, index++, creative_ad.per_month);
    BindInt(command, index++, creative_ad.total_max);
    BindDouble(command, index++, creative_ad.value);
    BindString(command, index++, creative_ad.split_test_group);
    BindString(command, index++, creative_ad.target_url);

    count++;
  }

  return count;
}

std::string CreativeAds::BuildInsertOrUpdateQuery(
    mojom::DBCommand* command,
    const CreativeAdList& creative_ads) {
  const int count = BindParameters(command, creative_ads);

  return base::StringPrintf(
      "INSERT OR REPLACE INTO %s "
      "(creative_instance_id, "
      "conversion, "
      "per_day, "
      "per_week, "
      "per_month, "
      "total_max, "
      "value, "
      "split_test_group, "
      "target_url) VALUES %s",
      GetTableName().c_str(),
      BuildBindingParameterPlaceholders(9, count).c_str());
}

void CreativeAds::CreateTableV16(mojom::DBTransaction* transaction) {
  DCHECK(transaction);

  const std::string query = base::StringPrintf(
      "CREATE TABLE %s "
      "(creative_instance_id TEXT NOT NULL PRIMARY KEY UNIQUE "
      "ON CONFLICT REPLACE, "
      "conversion INTEGER NOT NULL DEFAULT 0, "
      "per_day INTEGER NOT NULL DEFAULT 0, "
      "per_week INTEGER NOT NULL DEFAULT 0, "
      "per_month INTEGER NOT NULL DEFAULT 0, "
      "total_max INTEGER NOT NULL DEFAULT 0, "
      "value DOUBLE NOT NULL DEFAULT 0, "
      "split_test_group TEXT, "
      "target_url TEXT NOT NULL)",
      GetTableName().c_str());

  mojom::DBCommandPtr command = mojom::DBCommand::New();
  command->type = mojom::DBCommand::Type::EXECUTE;
  command->command = query;

  transaction->commands.push_back(std::move(command));
}

void CreativeAds::MigrateToV16(mojom::DBTransaction* transaction) {
  DCHECK(transaction);

  util::Drop(transaction, GetTableName());

  CreateTableV16(transaction);
}

}  // namespace table
}  // namespace database
}  // namespace ads
