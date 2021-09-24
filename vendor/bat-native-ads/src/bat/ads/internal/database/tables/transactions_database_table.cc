/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/database/tables/transactions_database_table.h"

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
const char kTableName[] = "transactions";
}  // namespace

Transactions::Transactions() = default;

Transactions::~Transactions() = default;

void Transactions::Save(const TransactionList& transactions,
                        ResultCallback callback) {
  if (transactions.empty()) {
    callback(/* success */ true);
    return;
  }

  mojom::DBTransactionPtr transaction = mojom::DBTransaction::New();

  InsertOrUpdate(transaction.get(), transactions);

  AdsClientHelper::Get()->RunDBTransaction(
      std::move(transaction),
      std::bind(&OnResultCallback, std::placeholders::_1, callback));
}

void Transactions::GetAll(GetTransactionsCallback callback) {
  const std::string query = base::StringPrintf(
      "SELECT "
      "created_at, "
      "value, "
      "confirmation_type, "
      "redeemed_at "
      "FROM %s",
      GetTableName().c_str());

  mojom::DBCommandPtr command = mojom::DBCommand::New();
  command->type = mojom::DBCommand::Type::READ;
  command->command = query;

  command->record_bindings = {
      mojom::DBCommand::RecordBindingType::DOUBLE_TYPE,  // created_at
      mojom::DBCommand::RecordBindingType::DOUBLE_TYPE,  // value
      mojom::DBCommand::RecordBindingType::STRING_TYPE,  // confirmation_type
      mojom::DBCommand::RecordBindingType::DOUBLE_TYPE   // redeemed_at
  };

  mojom::DBTransactionPtr transaction = mojom::DBTransaction::New();
  transaction->commands.push_back(std::move(command));

  AdsClientHelper::Get()->RunDBTransaction(
      std::move(transaction), std::bind(&Transactions::OnGetTransactions, this,
                                        std::placeholders::_1, callback));
}

std::string Transactions::GetTableName() const {
  return kTableName;
}

void Transactions::Migrate(mojom::DBTransaction* transaction,
                           const int to_version) {
  DCHECK(transaction);

  switch (to_version) {
    case 17: {
      MigrateToV17(transaction);
      break;
    }

    default: {
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

void Transactions::InsertOrUpdate(mojom::DBTransaction* transaction,
                                  const TransactionList& transactions) {
  DCHECK(transaction);

  if (transactions.empty()) {
    return;
  }

  mojom::DBCommandPtr command = mojom::DBCommand::New();
  command->type = mojom::DBCommand::Type::RUN;
  command->command = BuildInsertOrUpdateQuery(command.get(), transactions);

  transaction->commands.push_back(std::move(command));
}

int Transactions::BindParameters(mojom::DBCommand* command,
                                 const TransactionList& transactions) {
  DCHECK(command);

  int count = 0;

  int index = 0;
  for (const auto& transaction : transactions) {
    BindDouble(command, index++, transaction.created_at);
    BindDouble(command, index++, transaction.value);
    BindString(command, index++, transaction.confirmation_type);
    BindDouble(command, index++, transaction.redeemed_at);

    count++;
  }

  return count;
}

std::string Transactions::BuildInsertOrUpdateQuery(
    mojom::DBCommand* command,
    const TransactionList& transactions) {
  DCHECK(command);

  const int count = BindParameters(command, transactions);

  return base::StringPrintf(
      "INSERT OR REPLACE INTO %s "
      "(created_at, "
      "value, "
      "confirmation_type, "
      "redeemed_at) VALUES %s",
      GetTableName().c_str(),
      BuildBindingParameterPlaceholders(4, count).c_str());
}

void Transactions::OnGetTransactions(mojom::DBCommandResponsePtr response,
                                     GetTransactionsCallback callback) {
  if (!response ||
      response->status != mojom::DBCommandResponse::Status::RESPONSE_OK) {
    BLOG(0, "Failed to get transactions");
    callback(/* success */ false, {});
    return;
  }

  TransactionList transactions;

  for (const auto& record : response->result->get_records()) {
    TransactionInfo info = GetTransactionFromRecord(record.get());
    transactions.push_back(info);
  }

  callback(/* success */ true, transactions);
}

TransactionInfo Transactions::GetTransactionFromRecord(
    mojom::DBRecord* record) const {
  TransactionInfo info;
  info.created_at = ColumnDouble(record, 0);
  info.value = ColumnDouble(record, 1);
  info.confirmation_type = ColumnString(record, 2);
  info.redeemed_at = ColumnDouble(record, 3);

  return info;
}

void Transactions::CreateTableV17(mojom::DBTransaction* transaction) {
  DCHECK(transaction);

  const std::string query = base::StringPrintf(
      "CREATE TABLE transactions "
      "(id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
      "created_at TIMESTAMP NOT NULL, "
      "value DOUBLE NOT NULL, "
      "confirmation_type TEXT NOT NULL, "
      "redeemed_at TIMESTAMP)");

  mojom::DBCommandPtr command = mojom::DBCommand::New();
  command->type = mojom::DBCommand::Type::EXECUTE;
  command->command = query;

  transaction->commands.push_back(std::move(command));
}

void Transactions::MigrateToV17(mojom::DBTransaction* transaction) {
  DCHECK(transaction);

  CreateTableV17(transaction);
}

}  // namespace table
}  // namespace database
}  // namespace ads
