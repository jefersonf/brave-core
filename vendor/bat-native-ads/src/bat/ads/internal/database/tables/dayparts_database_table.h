/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_DATABASE_TABLES_DAYPARTS_DATABASE_TABLE_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_DATABASE_TABLES_DAYPARTS_DATABASE_TABLE_H_

#include <string>

#include "bat/ads/ads_client_aliases.h"
#include "bat/ads/internal/bundle/creative_ad_info_aliases.h"
#include "bat/ads/internal/database/database_table.h"
#include "bat/ads/public/interfaces/ads.mojom.h"

namespace ads {

namespace database {
namespace table {

class Dayparts final : public Table {
 public:
  Dayparts();
  ~Dayparts() override;

  void InsertOrUpdate(mojom::DBTransaction* transaction,
                      const CreativeAdList& creative_ads);

  void Delete(ResultCallback callback);

  std::string GetTableName() const override;

  void Migrate(mojom::DBTransaction* transaction,
               const int to_version) override;

 private:
  int BindParameters(mojom::DBCommand* command,
                     const CreativeAdList& creative_ads);

  std::string BuildInsertOrUpdateQuery(mojom::DBCommand* command,
                                       const CreativeAdList& creative_ads);

  void CreateTableV16(mojom::DBTransaction* transaction);
  void MigrateToV16(mojom::DBTransaction* transaction);
};

}  // namespace table
}  // namespace database
}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_DATABASE_TABLES_DAYPARTS_DATABASE_TABLE_H_
