/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/account/statement/statement.h"

#include "base/time/time.h"
#include "bat/ads/internal/account/transactions/transactions.h"
#include "bat/ads/statement_info.h"

namespace ads {

Statement::Statement() = default;

Statement::~Statement() = default;

StatementInfo Statement::Get(StatementCallback callback) const {
  database::table::Transactions database_table;
  database_table.GetAll([callback](const bool success,
                                   const TransactionList& transactions) {
    StatementInfo statement;

    statement.earnings_this_month = GetEarningsForThisMonth(transactions);

    statement.earnings_last_month = GetEarningsForLastMonth(transactions);

    statement.next_payment_date = GetNextPaymentDate(transactions);

    statement.ads_received_this_month =
        GetAdsReceivedForThisMonth(transactions);

    callback(success, statement);
  });
}

///////////////////////////////////////////////////////////////////////////////

base::Time GetNextPaymentDate(const TransactionList& transactions) const {
  // TODO(tmancey): Implement
}

double Statement::GetEarningsForThisMonth(
    const TransactionList& transactions) const {
  // TODO(tmancey): Implement
}

double Statement::GetEarningsForLastMonth(
    const TransactionList& transactions) const {
  // TODO(tmancey): Implement
}

int Statement::GetAdsReceivedForThisMonth(
    const TransactionList& transactions) const {
  // TODO(tmancey): Implement
}

}  // namespace ads
