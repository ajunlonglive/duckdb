//===----------------------------------------------------------------------===//
//                         DuckDB
//
// execution/operator/join/physical_cross_product.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "execution/physical_operator.hpp"
#include "common/types/chunk_collection.hpp"

namespace duckdb {
//! PhysicalCrossProduct represents a cross product between two tables
class PhysicalCrossProduct : public PhysicalOperator {
public:
	PhysicalCrossProduct(LogicalOperator &op, unique_ptr<PhysicalOperator> left, unique_ptr<PhysicalOperator> right);

	void _GetChunk(ClientContext &context, DataChunk &chunk, PhysicalOperatorState *state) override;

	unique_ptr<PhysicalOperatorState> GetOperatorState(ExpressionExecutor *parent_executor) override;
};

class PhysicalCrossProductOperatorState : public PhysicalOperatorState {
public:
	PhysicalCrossProductOperatorState(PhysicalOperator *left, PhysicalOperator *right,
	                                  ExpressionExecutor *parent_executor)
	    : PhysicalOperatorState(left, parent_executor), left_position(0) {
		assert(left && right);
	}

	size_t left_position;
	size_t right_position;
	ChunkCollection right_data;
};
} // namespace duckdb
