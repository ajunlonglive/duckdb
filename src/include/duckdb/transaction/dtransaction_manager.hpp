//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/transaction/transaction_manager.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/transaction/transaction_manager.hpp"
namespace duckdb {

//! The Transaction Manager is responsible for creating and managing
//! transactions
class DTransactionManager : public TransactionManager {
	friend struct CheckpointLock;

public:
	explicit DTransactionManager(AttachedDatabase &db);

public:
	//! Start a new transaction
	Transaction *StartTransaction(ClientContext &context) override;
	//! Commit the given transaction
	string CommitTransaction(ClientContext &context, Transaction *transaction) override;
	//! Rollback the given transaction
	void RollbackTransaction(Transaction *transaction) override;

	void Checkpoint(ClientContext &context, bool force = false) override;

	transaction_t LowestActiveId() {
		return lowest_active_id;
	}
	transaction_t LowestActiveStart() {
		return lowest_active_start;
	}

	bool IsDTransactionManager() override {
		return false;
	}

private:
	bool CanCheckpoint(Transaction *current = nullptr);
	//! Remove the given transaction from the list of active transactions
	void RemoveTransaction(Transaction *transaction) noexcept;
	void LockClients(vector<ClientLockWrapper> &client_locks, ClientContext &context);

private:
	//! The current start timestamp used by transactions
	transaction_t current_start_timestamp;
	//! The current transaction ID used by transactions
	transaction_t current_transaction_id;
	//! The lowest active transaction id
	atomic<transaction_t> lowest_active_id;
	//! The lowest active transaction timestamp
	atomic<transaction_t> lowest_active_start;
	//! Set of currently running transactions
	vector<unique_ptr<Transaction>> active_transactions;
	//! Set of recently committed transactions
	vector<unique_ptr<Transaction>> recently_committed_transactions;
	//! Transactions awaiting GC
	vector<unique_ptr<Transaction>> old_transactions;
	//! The lock used for transaction operations
	mutex transaction_lock;

	bool thread_is_checkpointing;
};

} // namespace duckdb
