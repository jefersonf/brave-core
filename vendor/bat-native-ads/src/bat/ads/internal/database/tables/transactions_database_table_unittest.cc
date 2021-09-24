/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/database/tables/transactions_database_table.h"

#include <memory>

#include "bat/ads/internal/container_util.h"
#include "bat/ads/internal/unittest_base.h"
#include "bat/ads/internal/unittest_time_util.h"
#include "bat/ads/internal/unittest_util.h"

// npm run test -- brave_unit_tests --filter=BatAds*

namespace ads {

class BatAdsTransactionsDatabaseTableTest : public UnitTestBase {
 protected:
  BatAdsTransactionsDatabaseTableTest()
      : database_table_(std::make_unique<database::table::Transactions>()) {}

  ~BatAdsTransactionsDatabaseTableTest() override = default;

  void Save(const TransactionList& transactions) {
    database_table_->Save(transactions,
                          [](const bool success) { ASSERT_TRUE(success); });
  }

  base::Time CalculateExpireAtTime(const int observation_window) {
    return Now() + base::TimeDelta::FromDays(observation_window);
  }

  std::unique_ptr<database::table::Transactions> database_table_;
};

TEST_F(BatAdsTransactionsDatabaseTableTest, EmptySave) {
  // Arrange
  TransactionList transactions = {};

  // Act
  Save(Transactions);

  // Assert
  const TransactionList expected_transactions = transactions;

  database_table_->GetAll(
      [&expected_transactions](const bool success,
                              const TransactionList& transactions) {
        ASSERT_TRUE(success);
        EXPECT_TRUE(CompareAsSets(expected_transactions, transactions));
      });
}

TEST_F(BatAdsTransactionsDatabaseTableTest, SaveTransactions) {
  // Arrange
  TransactionList transactions;

  TransactionInfo info_1;
  info_1.creative_set_id = "3519f52c-46a4-4c48-9c2b-c264c0067f04";
  info_1.type = "postview";
  info_1.url_pattern = "https://www.brave.com/*";
  info_1.observation_window = 3;
  info_1.expire_at = CalculateExpireAtTime(info_1.observation_window);
  transactions.push_back(info_1);

  TransactionInfo info_2;
  info_2.creative_set_id = "eaa6224a-46a4-4c48-9c2b-c264c0067f04";
  info_2.type = "postclick";
  info_2.url_pattern = "https://www.brave.com/signup/*";
  info_2.observation_window = 30;
  info_2.expire_at = CalculateExpireAtTime(info_2.observation_window);
  transactions.push_back(info_2);

  // Act
  Save(transactions);

  // Assert
  const TransactionList expected_transactions = transactions;

  database_table_->GetAll(
      [&expected_transactions](const bool success,
                               const TransactionList& transactions) {
        ASSERT_TRUE(success);
        EXPECT_TRUE(CompareAsSets(expected_transactions, transactions));
      });
}

TEST_F(BatAdsTransactionsDatabaseTableTest, DoNotSaveDuplicateTransaction) {
  // Arrange
  TransactionList transactions;

  TransactionInfo info;
  info.creative_set_id = "3519f52c-46a4-4c48-9c2b-c264c0067f04";
  info.type = "postview";
  info.url_pattern = "https://www.brave.com/*";
  info.observation_window = 3;
  info.expire_at = CalculateExpireAtTime(info.observation_window);
  transactions.push_back(info);

  Save(Transactions);

  // Act
  Save(Transactions);

  // Assert
  const TransactionList expected_transactions = transactions;

  database_table_->GetAll(
      [&expected_transactions](const bool success,
                               const TransactionList& transactions) {
        ASSERT_TRUE(success);
        EXPECT_TRUE(CompareAsSets(expected_transactions, transactions));
      });
}

TEST_F(BatAdsTransactionsDatabaseTableTest,
       SaveTransactionWithMatchingCreativeSetIdAndTypeAndUrlPattern) {
  // Arrange
  TransactionList transactions;

  TransactionInfo info_1;
  info_1.creative_set_id = "3519f52c-46a4-4c48-9c2b-c264c0067f04";
  info_1.type = "postview";
  info_1.url_pattern = "https://www.brave.com/*";
  info_1.observation_window = 3;
  info_1.expire_at = CalculateExpireAtTime(info_1.observation_window);
  transactions.push_back(info_1);

  Save(transactions);

  // Act
  TransactionInfo info_2;  // Should supersede info_1
  info_2.creative_set_id = "3519f52c-46a4-4c48-9c2b-c264c0067f04";
  info_2.type = "postview";
  info_2.url_pattern = "https://www.brave.com/*";
  info_2.observation_window = 30;
  info_2.expire_at = CalculateExpireAtTime(info_2.observation_window);
  transactions.push_back(info_2);

  Save(transactions);

  // Assert
  TransactionList expected_transactions;
  expected_transactions.push_back(info_2);

  database_table_->GetAll(
      [&expected_transactions](const bool success,
                               const TransactionList& transactions) {
        ASSERT_TRUE(success);
        EXPECT_TRUE(CompareAsSets(expected_transactions, transactions));
      });
}

TEST_F(BatAdsTransactionsDatabaseTableTest, TableName) {
  // Arrange

  // Act
  const std::string table_name = database_table_->GetTableName();

  // Assert
  const std::string expected_table_name = "transactions";
  EXPECT_EQ(expected_table_name, table_name);
}

}  // namespace ads
