//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/verification/prepared_statement_verifier.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/verification/statement_verifier.hpp"

namespace duckdb {

class PreparedStatementVerifier : public StatementVerifier {
public:
	explicit PreparedStatementVerifier(unique_ptr<SQLStatement> statement_p);
	static StatementVerifier Create(const SQLStatement &statement_p);
};

} // namespace duckdb
